/**
 * @file dmusicpak_c.cpp
 * @brief C wrapper implementation for DMusicPak library
 * 
 * This file provides C-compatible wrappers for the C++ API,
 * allowing C programs to use the library without C++ dependencies.
 */

#include "../include/dmusicpak/dmusicpak.h"
#include "../include/dmusicpak/dmusicpak_c.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

using namespace dmusicpak;

/* Helper functions to convert between C and C++ types */

static Error cpp_error_from_c(dmusicpak_error_t error) {
    return static_cast<Error>(error);
}

static dmusicpak_error_t c_error_from_cpp(Error error) {
    return static_cast<dmusicpak_error_t>(error);
}

static LyricFormat cpp_lyric_format_from_c(dmusicpak_lyric_format_t format) {
    return static_cast<LyricFormat>(format);
}

static dmusicpak_lyric_format_t c_lyric_format_from_cpp(LyricFormat format) {
    return static_cast<dmusicpak_lyric_format_t>(format);
}

static CoverFormat cpp_cover_format_from_c(dmusicpak_cover_format_t format) {
    return static_cast<CoverFormat>(format);
}

static dmusicpak_cover_format_t c_cover_format_from_cpp(CoverFormat format) {
    return static_cast<dmusicpak_cover_format_t>(format);
}

static AudioFormat cpp_audio_format_from_c(dmusicpak_audio_format_t format) {
    return static_cast<AudioFormat>(format);
}

static dmusicpak_audio_format_t c_audio_format_from_cpp(AudioFormat format) {
    return static_cast<dmusicpak_audio_format_t>(format);
}

static void cpp_metadata_from_c(const dmusicpak_metadata_t* c_meta, Metadata* cpp_meta) {
    cpp_meta->title = c_meta->title;
    cpp_meta->artist = c_meta->artist;
    cpp_meta->album = c_meta->album;
    cpp_meta->genre = c_meta->genre;
    cpp_meta->year = c_meta->year;
    cpp_meta->comment = c_meta->comment;
    cpp_meta->duration_ms = c_meta->duration_ms;
    cpp_meta->bitrate = c_meta->bitrate;
    cpp_meta->sample_rate = c_meta->sample_rate;
    cpp_meta->channels = c_meta->channels;
}

static void c_metadata_from_cpp(const Metadata* cpp_meta, dmusicpak_metadata_t* c_meta) {
    c_meta->title = cpp_meta->title;
    c_meta->artist = cpp_meta->artist;
    c_meta->album = cpp_meta->album;
    c_meta->genre = cpp_meta->genre;
    c_meta->year = cpp_meta->year;
    c_meta->comment = cpp_meta->comment;
    c_meta->duration_ms = cpp_meta->duration_ms;
    c_meta->bitrate = cpp_meta->bitrate;
    c_meta->sample_rate = cpp_meta->sample_rate;
    c_meta->channels = cpp_meta->channels;
}

static void cpp_lyrics_from_c(const dmusicpak_lyrics_t* c_lyrics, Lyrics* cpp_lyrics) {
    cpp_lyrics->format = cpp_lyric_format_from_c(c_lyrics->format);
    cpp_lyrics->data = c_lyrics->data;
    cpp_lyrics->size = c_lyrics->size;
}

static void c_lyrics_from_cpp(const Lyrics* cpp_lyrics, dmusicpak_lyrics_t* c_lyrics) {
    c_lyrics->format = c_lyric_format_from_cpp(cpp_lyrics->format);
    c_lyrics->data = cpp_lyrics->data;
    c_lyrics->size = cpp_lyrics->size;
}

static void cpp_audio_from_c(const dmusicpak_audio_t* c_audio, Audio* cpp_audio) {
    cpp_audio->format = cpp_audio_format_from_c(c_audio->format);
    cpp_audio->source_filename = c_audio->source_filename;
    cpp_audio->data = c_audio->data;
    cpp_audio->size = c_audio->size;
}

static void c_audio_from_cpp(const Audio* cpp_audio, dmusicpak_audio_t* c_audio) {
    c_audio->format = c_audio_format_from_cpp(cpp_audio->format);
    c_audio->source_filename = cpp_audio->source_filename;
    c_audio->data = cpp_audio->data;
    c_audio->size = cpp_audio->size;
}

static void cpp_cover_from_c(const dmusicpak_cover_t* c_cover, Cover* cpp_cover) {
    cpp_cover->format = cpp_cover_format_from_c(c_cover->format);
    cpp_cover->data = c_cover->data;
    cpp_cover->size = c_cover->size;
    cpp_cover->width = c_cover->width;
    cpp_cover->height = c_cover->height;
}

static void c_cover_from_cpp(const Cover* cpp_cover, dmusicpak_cover_t* c_cover) {
    c_cover->format = c_cover_format_from_cpp(cpp_cover->format);
    c_cover->data = cpp_cover->data;
    c_cover->size = cpp_cover->size;
    c_cover->width = cpp_cover->width;
    c_cover->height = cpp_cover->height;
}

/* C API implementations */

const char* dmusicpak_version(void) {
    return dmusicpak::version();
}

const char* dmusicpak_error_string(dmusicpak_error_t error) {
    return dmusicpak::error_string(cpp_error_from_c(error));
}

dmusicpak_package_t dmusicpak_create(void) {
    return reinterpret_cast<dmusicpak_package_t>(dmusicpak::create());
}

dmusicpak_package_t dmusicpak_load(const char* filename) {
    return reinterpret_cast<dmusicpak_package_t>(dmusicpak::load(filename));
}

dmusicpak_package_t dmusicpak_load_memory(const uint8_t* data, size_t size) {
    return reinterpret_cast<dmusicpak_package_t>(dmusicpak::load_memory(data, size));
}

#ifdef DMUSICPAK_ENABLE_NETWORK
dmusicpak_package_t dmusicpak_load_url(const char* url, uint32_t timeout_ms) {
    return reinterpret_cast<dmusicpak_package_t>(dmusicpak::load_url(url, timeout_ms));
}

dmusicpak_package_t dmusicpak_load_url_stream(const char* url, uint32_t timeout_ms, size_t chunk_size) {
    return reinterpret_cast<dmusicpak_package_t>(dmusicpak::load_url_stream(url, timeout_ms, chunk_size));
}

int64_t dmusicpak_get_audio_chunk_url(
    const char* url,
    size_t offset,
    size_t size,
    uint8_t* buffer,
    uint32_t timeout_ms
) {
    return dmusicpak::get_audio_chunk_url(url, offset, size, buffer, timeout_ms);
}
#endif /* DMUSICPAK_ENABLE_NETWORK */

dmusicpak_error_t dmusicpak_save(dmusicpak_package_t package, const char* filename) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg) return DMUSICPAK_ERROR_INVALID_PARAM;
    return c_error_from_cpp(dmusicpak::save(pkg, filename));
}

dmusicpak_error_t dmusicpak_save_memory(dmusicpak_package_t package, uint8_t** buffer, size_t* size) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg) return DMUSICPAK_ERROR_INVALID_PARAM;
    return c_error_from_cpp(dmusicpak::save_memory(pkg, buffer, size));
}

void dmusicpak_free(dmusicpak_package_t package) {
    if (!package) return;
    Package* pkg = reinterpret_cast<Package*>(package);
    dmusicpak::free(pkg);
}

dmusicpak_error_t dmusicpak_set_metadata(dmusicpak_package_t package, const dmusicpak_metadata_t* metadata) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg || !metadata) return DMUSICPAK_ERROR_INVALID_PARAM;
    
    Metadata cpp_meta;
    cpp_metadata_from_c(metadata, &cpp_meta);
    Error result = dmusicpak::set_metadata(pkg, &cpp_meta);
    return c_error_from_cpp(result);
}

dmusicpak_error_t dmusicpak_get_metadata(dmusicpak_package_t package, dmusicpak_metadata_t* metadata) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg || !metadata) return DMUSICPAK_ERROR_INVALID_PARAM;
    
    Metadata cpp_meta;
    Error result = dmusicpak::get_metadata(pkg, &cpp_meta);
    if (result == Error::OK) {
        c_metadata_from_cpp(&cpp_meta, metadata);
    }
    return c_error_from_cpp(result);
}

dmusicpak_error_t dmusicpak_set_lyrics(dmusicpak_package_t package, const dmusicpak_lyrics_t* lyrics) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg || !lyrics) return DMUSICPAK_ERROR_INVALID_PARAM;
    
    Lyrics cpp_lyrics;
    cpp_lyrics_from_c(lyrics, &cpp_lyrics);
    Error result = dmusicpak::set_lyrics(pkg, &cpp_lyrics);
    return c_error_from_cpp(result);
}

dmusicpak_error_t dmusicpak_get_lyrics(dmusicpak_package_t package, dmusicpak_lyrics_t* lyrics) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg || !lyrics) return DMUSICPAK_ERROR_INVALID_PARAM;
    
    Lyrics cpp_lyrics;
    Error result = dmusicpak::get_lyrics(pkg, &cpp_lyrics);
    if (result == Error::OK) {
        c_lyrics_from_cpp(&cpp_lyrics, lyrics);
    }
    return c_error_from_cpp(result);
}

dmusicpak_error_t dmusicpak_set_audio(dmusicpak_package_t package, const dmusicpak_audio_t* audio) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg || !audio) return DMUSICPAK_ERROR_INVALID_PARAM;
    
    Audio cpp_audio;
    cpp_audio_from_c(audio, &cpp_audio);
    Error result = dmusicpak::set_audio(pkg, &cpp_audio);
    return c_error_from_cpp(result);
}

dmusicpak_error_t dmusicpak_get_audio(dmusicpak_package_t package, dmusicpak_audio_t* audio) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg || !audio) return DMUSICPAK_ERROR_INVALID_PARAM;
    
    Audio cpp_audio;
    Error result = dmusicpak::get_audio(pkg, &cpp_audio);
    if (result == Error::OK) {
        c_audio_from_cpp(&cpp_audio, audio);
    }
    return c_error_from_cpp(result);
}

dmusicpak_error_t dmusicpak_set_cover(dmusicpak_package_t package, const dmusicpak_cover_t* cover) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg || !cover) return DMUSICPAK_ERROR_INVALID_PARAM;
    
    Cover cpp_cover;
    cpp_cover_from_c(cover, &cpp_cover);
    Error result = dmusicpak::set_cover(pkg, &cpp_cover);
    return c_error_from_cpp(result);
}

dmusicpak_error_t dmusicpak_get_cover(dmusicpak_package_t package, dmusicpak_cover_t* cover) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg || !cover) return DMUSICPAK_ERROR_INVALID_PARAM;
    
    Cover cpp_cover;
    Error result = dmusicpak::get_cover(pkg, &cpp_cover);
    if (result == Error::OK) {
        c_cover_from_cpp(&cpp_cover, cover);
    }
    return c_error_from_cpp(result);
}

dmusicpak_error_t dmusicpak_stream_audio(
    dmusicpak_package_t package,
    dmusicpak_stream_callback_t callback,
    void* userdata
) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg || !callback) return DMUSICPAK_ERROR_INVALID_PARAM;
    
    StreamCallback cpp_callback = reinterpret_cast<StreamCallback>(callback);
    Error result = dmusicpak::stream_audio(pkg, cpp_callback, userdata);
    return c_error_from_cpp(result);
}

int64_t dmusicpak_get_audio_chunk(
    dmusicpak_package_t package,
    size_t offset,
    size_t size,
    uint8_t* buffer
) {
    Package* pkg = reinterpret_cast<Package*>(package);
    if (!pkg) return -1;
    return dmusicpak::get_audio_chunk(pkg, offset, size, buffer);
}

void dmusicpak_free_metadata(dmusicpak_metadata_t* metadata) {
    if (!metadata) return;
    
    /* Free all string fields */
    ::free(metadata->title);
    ::free(metadata->artist);
    ::free(metadata->album);
    ::free(metadata->genre);
    ::free(metadata->year);
    ::free(metadata->comment);
    
    memset(metadata, 0, sizeof(dmusicpak_metadata_t));
}

void dmusicpak_free_lyrics(dmusicpak_lyrics_t* lyrics) {
    if (!lyrics) return;
    
    /* Free lyrics data */
    ::free(lyrics->data);
    
    memset(lyrics, 0, sizeof(dmusicpak_lyrics_t));
}

void dmusicpak_free_audio(dmusicpak_audio_t* audio) {
    if (!audio) return;
    
    /* Free audio data and source filename */
    ::free(audio->source_filename);
    ::free(audio->data);
    
    memset(audio, 0, sizeof(dmusicpak_audio_t));
}

void dmusicpak_free_cover(dmusicpak_cover_t* cover) {
    if (!cover) return;
    
    /* Free cover image data */
    ::free(cover->data);
    
    memset(cover, 0, sizeof(dmusicpak_cover_t));
}

#ifdef __cplusplus
}
#endif

