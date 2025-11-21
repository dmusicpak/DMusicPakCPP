/**
 * @file network.cpp
 * @brief Network streaming support for DMusicPak library
 * Requires libcurl and ENABLE_NETWORK option
 */

#ifdef DMUSICPAK_ENABLE_NETWORK

#include "../include/dmusicpak/dmusicpak.h"
#include "internal.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace dmusicpak;

/* Initialize curl library (call once at startup) */
static bool curl_initialized = false;

/* Helper function to ensure curl is initialized */
static void ensure_curl_initialized() {
    if (!curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = true;
    }
}

/* Memory buffer structure for curl write callback */
struct MemoryBuffer {
    uint8_t* data;
    size_t size;
    size_t capacity;
    size_t offset;  /* For streaming reads */
};

/* Write callback for curl (renamed to avoid conflict with curl.h typedef) */
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryBuffer* mem = (MemoryBuffer*)userp;
    
    /* Expand buffer if needed */
    if (mem->offset + realsize > mem->capacity) {
        size_t new_capacity = mem->capacity * 2;
        if (new_capacity < mem->offset + realsize) {
            new_capacity = mem->offset + realsize + 1024;
        }
        
        uint8_t* new_data = (uint8_t*)realloc(mem->data, new_capacity);
        if (!new_data) {
            return 0; /* Signal error */
        }
        
        mem->data = new_data;
        mem->capacity = new_capacity;
    }
    
    memcpy(mem->data + mem->offset, contents, realsize);
    mem->offset += realsize;
    mem->size = mem->offset;
    
    return realsize;
}

/* Initialize curl (thread-safe) */
static CURL* init_curl_handle(const char* url, uint32_t timeout_ms) {
    ensure_curl_initialized();
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "DMusicPak/1.0.1");
    
    if (timeout_ms > 0) {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)timeout_ms);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (long)(timeout_ms / 3));
    }
    
    return curl;
}

/* Download entire file from URL */
Package* dmusicpak::load_url(const char* url, uint32_t timeout_ms) {
    if (!url) return NULL;
    if (timeout_ms == 0) timeout_ms = 30000; /* Default 30 seconds */
    
    /* Initialize curl */
    CURLcode res;
    CURL* curl = init_curl_handle(url, timeout_ms);
    if (!curl) return NULL;
    
    /* Initialize memory buffer */
    MemoryBuffer mem;
    mem.data = NULL;
    mem.size = 0;
    mem.capacity = 0;
    mem.offset = 0;
    
    /* Allocate initial buffer */
    mem.capacity = 64 * 1024; /* 64KB initial */
    mem.data = (uint8_t*)malloc(mem.capacity);
    if (!mem.data) {
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    /* Set write callback */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);
    
    /* Perform download */
    res = curl_easy_perform(curl);
    
    /* Check HTTP response code */
    long http_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code < 200 || http_code >= 300) {
            res = CURLE_HTTP_RETURNED_ERROR;
        }
    }
    
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        if (mem.data) ::free(mem.data);
        return NULL;
    }
    
    /* Parse downloaded data */
    Package* package = load_memory(mem.data, mem.size);
    ::free(mem.data);
    
    return package;
}

/* Stream and parse incrementally */
Package* dmusicpak::load_url_stream(const char* url, uint32_t timeout_ms, size_t chunk_size) {
    if (!url) return NULL;
    if (timeout_ms == 0) timeout_ms = 30000; /* Default 30 seconds */
    if (chunk_size == 0) chunk_size = 65536; /* Default 64KB */
    
    /* For now, use simple download approach */
    /* TODO: Implement true streaming parser that can parse chunks as they arrive */
    /* This requires modifying the parser to handle partial data */
    
    return load_url(url, timeout_ms);
}

/* Get audio chunk using HTTP Range request */
int64_t dmusicpak::get_audio_chunk_url(
    const char* url,
    size_t offset,
    size_t size,
    uint8_t* buffer,
    uint32_t timeout_ms
) {
    if (!url || !buffer || size == 0) return -1;
    if (timeout_ms == 0) timeout_ms = 30000; /* Default 30 seconds */
    
    CURL* curl = init_curl_handle(url, timeout_ms);
    if (!curl) return -1;
    
    /* Set up Range request */
    char range_header[64];
    snprintf(range_header, sizeof(range_header), "Range: bytes=%zu-%zu", offset, offset + size - 1);
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, range_header);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    /* Set up write callback with fixed buffer */
    MemoryBuffer mem;
    mem.data = buffer;
    mem.size = 0;
    mem.capacity = size;
    mem.offset = 0;
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);
    
    /* Perform request */
    CURLcode res = curl_easy_perform(curl);
    
    long http_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        /* 206 is Partial Content (expected for Range requests) */
        if (http_code != 206 && (http_code < 200 || http_code >= 300)) {
            res = CURLE_HTTP_RETURNED_ERROR;
        }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return -1;
    }
    
    return (int64_t)mem.size;
}

/* Note: We rely on the OS to clean up curl on program exit */
/* For production use, consider adding a cleanup function */

#endif /* DMUSICPAK_ENABLE_NETWORK */

