/**
 * @file io.cpp
 * @brief File I/O operations for DMusicPak format
 */

#include "../include/dmusicpak/dmusicpak.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* File format constants */
#define DMUSICPAK_MAGIC "DMPK"
#define DMUSICPAK_VERSION 1

/* Chunk type identifiers */
#define CHUNK_METADATA 0x01
#define CHUNK_LYRICS   0x02
#define CHUNK_AUDIO    0x03
#define CHUNK_COVER    0x04

/* File header structure */
typedef struct {
    char magic[4];      /* "DMPK" */
    uint32_t version;   /* Format version */
    uint32_t num_chunks;/* Number of data chunks */
} file_header_t;

/* Chunk header structure */
typedef struct {
    uint8_t type;       /* Chunk type */
    uint32_t size;      /* Chunk data size */
} chunk_header_t;

/* Write string to buffer */
static size_t write_string(uint8_t* buffer, const char* str) {
    if (!str) {
        write_uint32_le(buffer, 0);
        return 4;
    }

    uint32_t len = (uint32_t)strlen(str);
    write_uint32_le(buffer, len);
    if (len > 0) {
        memcpy(buffer + 4, str, len);
    }
    return 4 + len;
}

/* Read string from buffer */
static size_t read_string(const uint8_t* buffer, char** str) {
    uint32_t len = read_uint32_le(buffer);
    if (len == 0) {
        *str = NULL;
        return 4;
    }

    *str = (char*)malloc(len + 1);
    if (!*str) return 0;

    memcpy(*str, buffer + 4, len);
    (*str)[len] = '\0';
    return 4 + len;
}

/* Calculate metadata chunk size */
static size_t calculate_metadata_size(const dmusicpak_metadata_t* metadata) {
    size_t size = 0;
    size += 4 + (metadata->title ? strlen(metadata->title) : 0);
    size += 4 + (metadata->artist ? strlen(metadata->artist) : 0);
    size += 4 + (metadata->album ? strlen(metadata->album) : 0);
    size += 4 + (metadata->genre ? strlen(metadata->genre) : 0);
    size += 4 + (metadata->year ? strlen(metadata->year) : 0);
    size += 4 + (metadata->comment ? strlen(metadata->comment) : 0);
    size += 4 + 4 + 4 + 2; /* duration_ms, bitrate, sample_rate, channels */
    return size;
}

/* Write metadata chunk */
static size_t write_metadata_chunk(uint8_t* buffer, const dmusicpak_metadata_t* metadata) {
    size_t offset = 0;

    offset += write_string(buffer + offset, metadata->title);
    offset += write_string(buffer + offset, metadata->artist);
    offset += write_string(buffer + offset, metadata->album);
    offset += write_string(buffer + offset, metadata->genre);
    offset += write_string(buffer + offset, metadata->year);
    offset += write_string(buffer + offset, metadata->comment);

    write_uint32_le(buffer + offset, metadata->duration_ms);
    offset += 4;
    write_uint32_le(buffer + offset, metadata->bitrate);
    offset += 4;
    write_uint32_le(buffer + offset, metadata->sample_rate);
    offset += 4;
    write_uint16_le(buffer + offset, metadata->channels);
    offset += 2;

    return offset;
}

/* Read metadata chunk */
static size_t read_metadata_chunk(const uint8_t* buffer, dmusicpak_metadata_t* metadata) {
    size_t offset = 0;

    offset += read_string(buffer + offset, &metadata->title);
    offset += read_string(buffer + offset, &metadata->artist);
    offset += read_string(buffer + offset, &metadata->album);
    offset += read_string(buffer + offset, &metadata->genre);
    offset += read_string(buffer + offset, &metadata->year);
    offset += read_string(buffer + offset, &metadata->comment);

    metadata->duration_ms = read_uint32_le(buffer + offset);
    offset += 4;
    metadata->bitrate = read_uint32_le(buffer + offset);
    offset += 4;
    metadata->sample_rate = read_uint32_le(buffer + offset);
    offset += 4;
    metadata->channels = read_uint16_le(buffer + offset);
    offset += 2;

    return offset;
}

dmusicpak_error_t dmusicpak_save(dmusicpak_package_t* package, const char* filename) {
    if (!package || !filename) return DMUSICPAK_ERROR_INVALID_PARAM;

    uint8_t* buffer = NULL;
    size_t size = 0;
    dmusicpak_error_t result = dmusicpak_save_memory(package, &buffer, &size);
    if (result != DMUSICPAK_OK) return result;

    FILE* file = fopen(filename, "wb");
    if (!file) {
        free(buffer);
        return DMUSICPAK_ERROR_FILE_NOT_FOUND;
    }

    size_t written = fwrite(buffer, 1, size, file);
    fclose(file);
    free(buffer);

    return (written == size) ? DMUSICPAK_OK : DMUSICPAK_ERROR_IO;
}

dmusicpak_error_t dmusicpak_save_memory(dmusicpak_package_t* package, uint8_t** buffer, size_t* size) {
    if (!package || !buffer || !size) return DMUSICPAK_ERROR_INVALID_PARAM;

    /* Calculate total size */
    size_t total_size = sizeof(file_header_t);
    uint32_t num_chunks = 0;

    if (package->has_metadata) {
        total_size += sizeof(chunk_header_t) + calculate_metadata_size(&package->metadata);
        num_chunks++;
    }
    if (package->has_lyrics) {
        total_size += sizeof(chunk_header_t) + 4 + package->lyrics.size;
        num_chunks++;
    }
    if (package->has_audio) {
        total_size += sizeof(chunk_header_t) + 4 +
                     (package->audio.source_filename ? strlen(package->audio.source_filename) : 0) +
                     package->audio.size;
        num_chunks++;
    }
    if (package->has_cover) {
        total_size += sizeof(chunk_header_t) + 4 + 4 + 4 + package->cover.size;
        num_chunks++;
    }

    /* Allocate buffer */
    *buffer = (uint8_t*)malloc(total_size);
    if (!*buffer) return DMUSICPAK_ERROR_MEMORY_ALLOC;

    size_t offset = 0;

    /* Write file header */
    memcpy(*buffer + offset, DMUSICPAK_MAGIC, 4);
    offset += 4;
    write_uint32_le(*buffer + offset, DMUSICPAK_VERSION);
    offset += 4;
    write_uint32_le(*buffer + offset, num_chunks);
    offset += 4;

    /* Write metadata chunk */
    if (package->has_metadata) {
        (*buffer)[offset++] = CHUNK_METADATA;
        uint32_t chunk_size = (uint32_t)calculate_metadata_size(&package->metadata);
        write_uint32_le(*buffer + offset, chunk_size);
        offset += 4;
        offset += write_metadata_chunk(*buffer + offset, &package->metadata);
    }

    /* Write lyrics chunk */
    if (package->has_lyrics) {
        (*buffer)[offset++] = CHUNK_LYRICS;
        uint32_t chunk_size = 4 + package->lyrics.size;
        write_uint32_le(*buffer + offset, chunk_size);
        offset += 4;
        write_uint32_le(*buffer + offset, (uint32_t)package->lyrics.format);
        offset += 4;
        memcpy(*buffer + offset, package->lyrics.data, package->lyrics.size);
        offset += package->lyrics.size;
    }

    /* Write audio chunk */
    if (package->has_audio) {
        (*buffer)[offset++] = CHUNK_AUDIO;
        uint32_t filename_len = package->audio.source_filename ? strlen(package->audio.source_filename) : 0;
        uint32_t chunk_size = 4 + filename_len + package->audio.size;
        write_uint32_le(*buffer + offset, chunk_size);
        offset += 4;
        offset += write_string(*buffer + offset, package->audio.source_filename);
        memcpy(*buffer + offset, package->audio.data, package->audio.size);
        offset += package->audio.size;
    }

    /* Write cover chunk */
    if (package->has_cover) {
        (*buffer)[offset++] = CHUNK_COVER;
        uint32_t chunk_size = 4 + 4 + 4 + package->cover.size;
        write_uint32_le(*buffer + offset, chunk_size);
        offset += 4;
        write_uint32_le(*buffer + offset, (uint32_t)package->cover.format);
        offset += 4;
        write_uint32_le(*buffer + offset, package->cover.width);
        offset += 4;
        write_uint32_le(*buffer + offset, package->cover.height);
        offset += 4;
        memcpy(*buffer + offset, package->cover.data, package->cover.size);
        offset += package->cover.size;
    }

    *size = offset;
    return DMUSICPAK_OK;
}

dmusicpak_package_t* dmusicpak_load(const char* filename) {
    if (!filename) return NULL;

    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;

    /* Get file size */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(file);
        return NULL;
    }

    /* Read entire file */
    uint8_t* buffer = (uint8_t*)malloc(file_size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (read != (size_t)file_size) {
        free(buffer);
        return NULL;
    }

    dmusicpak_package_t* package = dmusicpak_load_memory(buffer, file_size);
    free(buffer);

    return package;
}

dmusicpak_package_t* dmusicpak_load_memory(const uint8_t* data, size_t size) {
    if (!data || size < sizeof(file_header_t)) return NULL;

    /* Verify magic number */
    if (memcmp(data, DMUSICPAK_MAGIC, 4) != 0) return NULL;

    size_t offset = 4;
    uint32_t version = read_uint32_le(data + offset);
    offset += 4;

    if (version != DMUSICPAK_VERSION) return NULL;

    uint32_t num_chunks = read_uint32_le(data + offset);
    offset += 4;

    dmusicpak_package_t* package = dmusicpak_create();
    if (!package) return NULL;

    /* Read chunks */
    for (uint32_t i = 0; i < num_chunks && offset < size; i++) {
        if (offset + 5 > size) break;

        uint8_t chunk_type = data[offset++];
        uint32_t chunk_size = read_uint32_le(data + offset);
        offset += 4;

        if (offset + chunk_size > size) break;

        switch (chunk_type) {
            case CHUNK_METADATA:
                read_metadata_chunk(data + offset, &package->metadata);
                package->has_metadata = 1;
                break;

            case CHUNK_LYRICS:
                package->lyrics.format = (lyric_format_t)read_uint32_le(data + offset);
                package->lyrics.size = chunk_size - 4;
                if (package->lyrics.size > 0) {
                    package->lyrics.data = (uint8_t*)malloc(package->lyrics.size);
                    if (package->lyrics.data) {
                        memcpy(package->lyrics.data, data + offset + 4, package->lyrics.size);
                        package->has_lyrics = 1;
                    }
                }
                break;

            case CHUNK_AUDIO: {
                size_t str_offset = 0;
                str_offset += read_string(data + offset, &package->audio.source_filename);
                package->audio.size = chunk_size - str_offset;
                if (package->audio.size > 0) {
                    package->audio.data = (uint8_t*)malloc(package->audio.size);
                    if (package->audio.data) {
                        memcpy(package->audio.data, data + offset + str_offset, package->audio.size);
                        package->has_audio = 1;
                    }
                }
                break;
            }

            case CHUNK_COVER:
                package->cover.format = (cover_format_t)read_uint32_le(data + offset);
                package->cover.width = read_uint32_le(data + offset + 4);
                package->cover.height = read_uint32_le(data + offset + 8);
                package->cover.size = chunk_size - 12;
                if (package->cover.size > 0) {
                    package->cover.data = (uint8_t*)malloc(package->cover.size);
                    if (package->cover.data) {
                        memcpy(package->cover.data, data + offset + 12, package->cover.size);
                        package->has_cover = 1;
                    }
                }
                break;
        }

        offset += chunk_size;
    }

    return package;
}