/**
 * @file dmusicpak.cpp
 * @brief Implementation of DMusicPak library core functions
 */

#include "../include/dmusicpak/dmusicpak.h"
#include "internal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* File format constants */
#define DMUSICPAK_MAGIC "DMPK"
#define DMUSICPAK_VERSION 1

/* Chunk type identifiers */
#define CHUNK_METADATA 0x01
#define CHUNK_LYRICS   0x02
#define CHUNK_AUDIO    0x03
#define CHUNK_COVER    0x04

/* Helper function to duplicate string */
static char* str_dup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* dup = (char*)malloc(len + 1);
    if (dup) {
        memcpy(dup, str, len + 1);
    }
    return dup;
}

/* Implement helper functions for endian conversion */
void write_uint32_le(uint8_t* buffer, uint32_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
    buffer[2] = (uint8_t)((value >> 16) & 0xFF);
    buffer[3] = (uint8_t)((value >> 24) & 0xFF);
}

uint32_t read_uint32_le(const uint8_t* buffer) {
    return ((uint32_t)buffer[0]) |
           ((uint32_t)buffer[1] << 8) |
           ((uint32_t)buffer[2] << 16) |
           ((uint32_t)buffer[3] << 24);
}

void write_uint16_le(uint8_t* buffer, uint16_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
}

uint16_t read_uint16_le(const uint8_t* buffer) {
    return ((uint16_t)buffer[0]) | ((uint16_t)buffer[1] << 8);
}

const char* dmusicpak_version(void) {
    return "1.0.1";
}

const char* dmusicpak_error_string(dmusicpak_error_t error) {
    switch (error) {
        case DMUSICPAK_OK: return "Success";
        case DMUSICPAK_ERROR_INVALID_PARAM: return "Invalid parameter";
        case DMUSICPAK_ERROR_FILE_NOT_FOUND: return "File not found";
        case DMUSICPAK_ERROR_INVALID_FORMAT: return "Invalid format";
        case DMUSICPAK_ERROR_MEMORY_ALLOC: return "Memory allocation failed";
        case DMUSICPAK_ERROR_IO: return "I/O error";
        case DMUSICPAK_ERROR_NOT_SUPPORTED: return "Not supported";
        case DMUSICPAK_ERROR_CORRUPTED: return "File corrupted";
        default: return "Unknown error";
    }
}

dmusicpak_package_t* dmusicpak_create(void) {
    dmusicpak_package_t* package = (dmusicpak_package_t*)calloc(1, sizeof(dmusicpak_package_t));
    if (!package) return NULL;

    memset(package, 0, sizeof(dmusicpak_package_t));
    return package;
}

void dmusicpak_free(dmusicpak_package_t* package) {
    if (!package) return;

    dmusicpak_free_metadata(&package->metadata);
    dmusicpak_free_lyrics(&package->lyrics);
    dmusicpak_free_audio(&package->audio);
    dmusicpak_free_cover(&package->cover);

    free(package);
}

dmusicpak_error_t dmusicpak_set_metadata(dmusicpak_package_t* package, const dmusicpak_metadata_t* metadata) {
    if (!package || !metadata) return DMUSICPAK_ERROR_INVALID_PARAM;

    /* Free existing metadata */
    dmusicpak_free_metadata(&package->metadata);

    /* Copy strings */
    package->metadata.title = str_dup(metadata->title);
    package->metadata.artist = str_dup(metadata->artist);
    package->metadata.album = str_dup(metadata->album);
    package->metadata.genre = str_dup(metadata->genre);
    package->metadata.year = str_dup(metadata->year);
    package->metadata.comment = str_dup(metadata->comment);

    /* Copy numeric values */
    package->metadata.duration_ms = metadata->duration_ms;
    package->metadata.bitrate = metadata->bitrate;
    package->metadata.sample_rate = metadata->sample_rate;
    package->metadata.channels = metadata->channels;

    package->has_metadata = 1;
    return DMUSICPAK_OK;
}

dmusicpak_error_t dmusicpak_get_metadata(dmusicpak_package_t* package, dmusicpak_metadata_t* metadata) {
    if (!package || !metadata) return DMUSICPAK_ERROR_INVALID_PARAM;
    if (!package->has_metadata) return DMUSICPAK_ERROR_NOT_SUPPORTED;

    memset(metadata, 0, sizeof(dmusicpak_metadata_t));

    metadata->title = str_dup(package->metadata.title);
    metadata->artist = str_dup(package->metadata.artist);
    metadata->album = str_dup(package->metadata.album);
    metadata->genre = str_dup(package->metadata.genre);
    metadata->year = str_dup(package->metadata.year);
    metadata->comment = str_dup(package->metadata.comment);
    metadata->duration_ms = package->metadata.duration_ms;
    metadata->bitrate = package->metadata.bitrate;
    metadata->sample_rate = package->metadata.sample_rate;
    metadata->channels = package->metadata.channels;

    return DMUSICPAK_OK;
}

dmusicpak_error_t dmusicpak_set_lyrics(dmusicpak_package_t* package, const dmusicpak_lyrics_t* lyrics) {
    if (!package || !lyrics) return DMUSICPAK_ERROR_INVALID_PARAM;

    dmusicpak_free_lyrics(&package->lyrics);

    package->lyrics.format = lyrics->format;
    package->lyrics.size = lyrics->size;

    if (lyrics->data && lyrics->size > 0) {
        package->lyrics.data = (uint8_t*)malloc(lyrics->size);
        if (!package->lyrics.data) return DMUSICPAK_ERROR_MEMORY_ALLOC;
        memcpy(package->lyrics.data, lyrics->data, lyrics->size);
    }

    package->has_lyrics = 1;
    return DMUSICPAK_OK;
}

dmusicpak_error_t dmusicpak_get_lyrics(dmusicpak_package_t* package, dmusicpak_lyrics_t* lyrics) {
    if (!package || !lyrics) return DMUSICPAK_ERROR_INVALID_PARAM;
    if (!package->has_lyrics) return DMUSICPAK_ERROR_NOT_SUPPORTED;

    memset(lyrics, 0, sizeof(dmusicpak_lyrics_t));

    lyrics->format = package->lyrics.format;
    lyrics->size = package->lyrics.size;

    if (package->lyrics.data && package->lyrics.size > 0) {
        lyrics->data = (uint8_t*)malloc(package->lyrics.size);
        if (!lyrics->data) return DMUSICPAK_ERROR_MEMORY_ALLOC;
        memcpy(lyrics->data, package->lyrics.data, package->lyrics.size);
    }

    return DMUSICPAK_OK;
}

dmusicpak_error_t dmusicpak_set_audio(dmusicpak_package_t* package, const dmusicpak_audio_t* audio) {
    if (!package || !audio) return DMUSICPAK_ERROR_INVALID_PARAM;

    dmusicpak_free_audio(&package->audio);

    package->audio.source_filename = str_dup(audio->source_filename);
    package->audio.size = audio->size;

    if (audio->data && audio->size > 0) {
        package->audio.data = (uint8_t*)malloc(audio->size);
        if (!package->audio.data) return DMUSICPAK_ERROR_MEMORY_ALLOC;
        memcpy(package->audio.data, audio->data, audio->size);
    }

    package->has_audio = 1;
    return DMUSICPAK_OK;
}

dmusicpak_error_t dmusicpak_get_audio(dmusicpak_package_t* package, dmusicpak_audio_t* audio) {
    if (!package || !audio) return DMUSICPAK_ERROR_INVALID_PARAM;
    if (!package->has_audio) return DMUSICPAK_ERROR_NOT_SUPPORTED;

    memset(audio, 0, sizeof(dmusicpak_audio_t));

    audio->source_filename = str_dup(package->audio.source_filename);
    audio->size = package->audio.size;

    if (package->audio.data && package->audio.size > 0) {
        audio->data = (uint8_t*)malloc(package->audio.size);
        if (!audio->data) return DMUSICPAK_ERROR_MEMORY_ALLOC;
        memcpy(audio->data, package->audio.data, package->audio.size);
    }

    return DMUSICPAK_OK;
}

dmusicpak_error_t dmusicpak_set_cover(dmusicpak_package_t* package, const dmusicpak_cover_t* cover) {
    if (!package || !cover) return DMUSICPAK_ERROR_INVALID_PARAM;

    dmusicpak_free_cover(&package->cover);

    package->cover.format = cover->format;
    package->cover.size = cover->size;
    package->cover.width = cover->width;
    package->cover.height = cover->height;

    if (cover->data && cover->size > 0) {
        package->cover.data = (uint8_t*)malloc(cover->size);
        if (!package->cover.data) return DMUSICPAK_ERROR_MEMORY_ALLOC;
        memcpy(package->cover.data, cover->data, cover->size);
    }

    package->has_cover = 1;
    return DMUSICPAK_OK;
}

dmusicpak_error_t dmusicpak_get_cover(dmusicpak_package_t* package, dmusicpak_cover_t* cover) {
    if (!package || !cover) return DMUSICPAK_ERROR_INVALID_PARAM;
    if (!package->has_cover) return DMUSICPAK_ERROR_NOT_SUPPORTED;

    memset(cover, 0, sizeof(dmusicpak_cover_t));

    cover->format = package->cover.format;
    cover->size = package->cover.size;
    cover->width = package->cover.width;
    cover->height = package->cover.height;

    if (package->cover.data && package->cover.size > 0) {
        cover->data = (uint8_t*)malloc(package->cover.size);
        if (!cover->data) return DMUSICPAK_ERROR_MEMORY_ALLOC;
        memcpy(cover->data, package->cover.data, package->cover.size);
    }

    return DMUSICPAK_OK;
}

dmusicpak_error_t dmusicpak_stream_audio(
    dmusicpak_package_t* package,
    dmusicpak_stream_callback_t callback,
    void* userdata
) {
    if (!package || !callback) return DMUSICPAK_ERROR_INVALID_PARAM;
    if (!package->has_audio) return DMUSICPAK_ERROR_NOT_SUPPORTED;

    const size_t chunk_size = 8192; /* 8KB chunks */
    size_t offset = 0;

    while (offset < package->audio.size) {
        size_t to_read = chunk_size;
        if (offset + to_read > package->audio.size) {
            to_read = package->audio.size - offset;
        }

        size_t written = callback(package->audio.data + offset, 1, to_read, userdata);
        if (written == 0) break;

        offset += written;
    }

    return DMUSICPAK_OK;
}

int64_t dmusicpak_get_audio_chunk(
    dmusicpak_package_t* package,
    size_t offset,
    size_t size,
    uint8_t* buffer
) {
    if (!package || !buffer) return -1;
    if (!package->has_audio) return -1;
    if (offset >= package->audio.size) return 0;

    size_t to_read = size;
    if (offset + to_read > package->audio.size) {
        to_read = package->audio.size - offset;
    }

    memcpy(buffer, package->audio.data + offset, to_read);
    return (int64_t)to_read;
}

void dmusicpak_free_metadata(dmusicpak_metadata_t* metadata) {
    if (!metadata) return;

    free(metadata->title);
    free(metadata->artist);
    free(metadata->album);
    free(metadata->genre);
    free(metadata->year);
    free(metadata->comment);

    memset(metadata, 0, sizeof(dmusicpak_metadata_t));
}

void dmusicpak_free_lyrics(dmusicpak_lyrics_t* lyrics) {
    if (!lyrics) return;

    free(lyrics->data);
    memset(lyrics, 0, sizeof(dmusicpak_lyrics_t));
}

void dmusicpak_free_audio(dmusicpak_audio_t* audio) {
    if (!audio) return;

    free(audio->source_filename);
    free(audio->data);
    memset(audio, 0, sizeof(dmusicpak_audio_t));
}

void dmusicpak_free_cover(dmusicpak_cover_t* cover) {
    if (!cover) return;

    free(cover->data);
    memset(cover, 0, sizeof(dmusicpak_cover_t));
}