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

using namespace dmusicpak;

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
void dmusicpak::write_uint32_le(uint8_t* buffer, uint32_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
    buffer[2] = (uint8_t)((value >> 16) & 0xFF);
    buffer[3] = (uint8_t)((value >> 24) & 0xFF);
}

uint32_t dmusicpak::read_uint32_le(const uint8_t* buffer) {
    return ((uint32_t)buffer[0]) |
           ((uint32_t)buffer[1] << 8) |
           ((uint32_t)buffer[2] << 16) |
           ((uint32_t)buffer[3] << 24);
}

void dmusicpak::write_uint16_le(uint8_t* buffer, uint16_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
}

uint16_t dmusicpak::read_uint16_le(const uint8_t* buffer) {
    return ((uint16_t)buffer[0]) | ((uint16_t)buffer[1] << 8);
}

const char* dmusicpak::version() {
    return "1.0.1";
}

const char* dmusicpak::error_string(Error error) {
    switch (error) {
        case Error::OK: return "Success";
        case Error::INVALID_PARAM: return "Invalid parameter";
        case Error::FILE_NOT_FOUND: return "File not found";
        case Error::INVALID_FORMAT: return "Invalid format";
        case Error::MEMORY_ALLOC: return "Memory allocation failed";
        case Error::IO: return "I/O error";
        case Error::NOT_SUPPORTED: return "Not supported";
        case Error::CORRUPTED: return "File corrupted";
        case Error::NETWORK: return "Network error";
        default: return "Unknown error";
    }
}

Package* dmusicpak::create() {
    Package* package = (Package*)calloc(1, sizeof(Package));
    if (!package) return NULL;

    memset(package, 0, sizeof(Package));
    return package;
}

void dmusicpak::free(Package* package) {
    if (!package) return;

    free_metadata(&package->metadata);
    free_lyrics(&package->lyrics);
    free_audio(&package->audio);
    free_cover(&package->cover);

    ::free(package);
}

Error dmusicpak::set_metadata(Package* package, const Metadata* metadata) {
    if (!package || !metadata) return Error::INVALID_PARAM;

    /* Free existing metadata */
    free_metadata(&package->metadata);

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
    return Error::OK;
}

Error dmusicpak::get_metadata(Package* package, Metadata* metadata) {
    if (!package || !metadata) return Error::INVALID_PARAM;
    if (!package->has_metadata) return Error::NOT_SUPPORTED;

    memset(metadata, 0, sizeof(Metadata));

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

    return Error::OK;
}

Error dmusicpak::set_lyrics(Package* package, const Lyrics* lyrics) {
    if (!package || !lyrics) return Error::INVALID_PARAM;

    free_lyrics(&package->lyrics);

    package->lyrics.format = lyrics->format;
    package->lyrics.size = lyrics->size;

    if (lyrics->data && lyrics->size > 0) {
        package->lyrics.data = (uint8_t*)malloc(lyrics->size);
        if (!package->lyrics.data) return Error::MEMORY_ALLOC;
        memcpy(package->lyrics.data, lyrics->data, lyrics->size);
    }

    package->has_lyrics = 1;
    return Error::OK;
}

Error dmusicpak::get_lyrics(Package* package, Lyrics* lyrics) {
    if (!package || !lyrics) return Error::INVALID_PARAM;
    if (!package->has_lyrics) return Error::NOT_SUPPORTED;

    memset(lyrics, 0, sizeof(Lyrics));

    lyrics->format = package->lyrics.format;
    lyrics->size = package->lyrics.size;

    if (package->lyrics.data && package->lyrics.size > 0) {
        lyrics->data = (uint8_t*)malloc(package->lyrics.size);
        if (!lyrics->data) return Error::MEMORY_ALLOC;
        memcpy(lyrics->data, package->lyrics.data, package->lyrics.size);
    }

    return Error::OK;
}

Error dmusicpak::set_audio(Package* package, const Audio* audio) {
    if (!package || !audio) return Error::INVALID_PARAM;

    free_audio(&package->audio);

    package->audio.format = audio->format;
    package->audio.source_filename = str_dup(audio->source_filename);
    package->audio.size = audio->size;

    if (audio->data && audio->size > 0) {
        package->audio.data = (uint8_t*)malloc(audio->size);
        if (!package->audio.data) return Error::MEMORY_ALLOC;
        memcpy(package->audio.data, audio->data, audio->size);
    }

    package->has_audio = 1;
    return Error::OK;
}

Error dmusicpak::get_audio(Package* package, Audio* audio) {
    if (!package || !audio) return Error::INVALID_PARAM;
    if (!package->has_audio) return Error::NOT_SUPPORTED;

    memset(audio, 0, sizeof(Audio));

    audio->format = package->audio.format;
    audio->source_filename = str_dup(package->audio.source_filename);
    audio->size = package->audio.size;

    if (package->audio.data && package->audio.size > 0) {
        audio->data = (uint8_t*)malloc(package->audio.size);
        if (!audio->data) return Error::MEMORY_ALLOC;
        memcpy(audio->data, package->audio.data, package->audio.size);
    }

    return Error::OK;
}

Error dmusicpak::set_cover(Package* package, const Cover* cover) {
    if (!package || !cover) return Error::INVALID_PARAM;

    free_cover(&package->cover);

    package->cover.format = cover->format;
    package->cover.size = cover->size;
    package->cover.width = cover->width;
    package->cover.height = cover->height;

    if (cover->data && cover->size > 0) {
        package->cover.data = (uint8_t*)malloc(cover->size);
        if (!package->cover.data) return Error::MEMORY_ALLOC;
        memcpy(package->cover.data, cover->data, cover->size);
    }

    package->has_cover = 1;
    return Error::OK;
}

Error dmusicpak::get_cover(Package* package, Cover* cover) {
    if (!package || !cover) return Error::INVALID_PARAM;
    if (!package->has_cover) return Error::NOT_SUPPORTED;

    memset(cover, 0, sizeof(Cover));

    cover->format = package->cover.format;
    cover->size = package->cover.size;
    cover->width = package->cover.width;
    cover->height = package->cover.height;

    if (package->cover.data && package->cover.size > 0) {
        cover->data = (uint8_t*)malloc(package->cover.size);
        if (!cover->data) return Error::MEMORY_ALLOC;
        memcpy(cover->data, package->cover.data, package->cover.size);
    }

    return Error::OK;
}

Error dmusicpak::stream_audio(
    Package* package,
    StreamCallback callback,
    void* userdata
) {
    if (!package || !callback) return Error::INVALID_PARAM;
    if (!package->has_audio) return Error::NOT_SUPPORTED;

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

    return Error::OK;
}

int64_t dmusicpak::get_audio_chunk(
    Package* package,
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

void dmusicpak::free_metadata(Metadata* metadata) {
    if (!metadata) return;

    ::free(metadata->title);
    ::free(metadata->artist);
    ::free(metadata->album);
    ::free(metadata->genre);
    ::free(metadata->year);
    ::free(metadata->comment);

    memset(metadata, 0, sizeof(Metadata));
}

void dmusicpak::free_lyrics(Lyrics* lyrics) {
    if (!lyrics) return;

    ::free(lyrics->data);
    memset(lyrics, 0, sizeof(Lyrics));
}

void dmusicpak::free_audio(Audio* audio) {
    if (!audio) return;

    ::free(audio->source_filename);
    ::free(audio->data);
    memset(audio, 0, sizeof(Audio));
}

void dmusicpak::free_cover(Cover* cover) {
    if (!cover) return;

    ::free(cover->data);
    memset(cover, 0, sizeof(Cover));
}
