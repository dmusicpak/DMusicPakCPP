/**
 * @file io.cpp
 * @brief File I/O operations for DMusicPak format
 */

#include "../include/dmusicpak/dmusicpak.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace dmusicpak;

/* File format constants */
#define DMUSICPAK_MAGIC "DMPK"
#define DMUSICPAK_VERSION 2

/* Chunk type identifiers */
#define CHUNK_METADATA 0x01
#define CHUNK_LYRICS   0x02
#define CHUNK_AUDIO    0x03
#define CHUNK_COVER    0x04
#define FILE_HEADER_SIZE 12
#define CHUNK_HEADER_SIZE 5

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
static bool read_string(const uint8_t* buffer, size_t available, char** str, size_t* consumed) {
    *str = NULL;
    *consumed = 0;
    if (available < 4) return false;

    uint32_t len = read_uint32_le(buffer);
    if ((size_t)len > available - 4) return false;

    if (len == 0) {
        *consumed = 4;
        return true;
    }

    *str = (char*)malloc(len + 1);
    if (!*str) return false;

    memcpy(*str, buffer + 4, len);
    (*str)[len] = '\0';
    *consumed = 4 + (size_t)len;
    return true;
}

/* Calculate metadata chunk size */
static size_t calculate_metadata_size(const Metadata* metadata) {
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
static size_t write_metadata_chunk(uint8_t* buffer, const Metadata* metadata) {
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
static bool read_metadata_chunk(
    const uint8_t* buffer,
    size_t available,
    Metadata* metadata,
    size_t* consumed
) {
    size_t offset = 0;
    size_t field_size = 0;

    if (!read_string(buffer + offset, available - offset, &metadata->title, &field_size)) return false;
    offset += field_size;
    if (!read_string(buffer + offset, available - offset, &metadata->artist, &field_size)) return false;
    offset += field_size;
    if (!read_string(buffer + offset, available - offset, &metadata->album, &field_size)) return false;
    offset += field_size;
    if (!read_string(buffer + offset, available - offset, &metadata->genre, &field_size)) return false;
    offset += field_size;
    if (!read_string(buffer + offset, available - offset, &metadata->year, &field_size)) return false;
    offset += field_size;
    if (!read_string(buffer + offset, available - offset, &metadata->comment, &field_size)) return false;
    offset += field_size;

    if (available - offset < 14) return false;

    metadata->duration_ms = read_uint32_le(buffer + offset);
    offset += 4;
    metadata->bitrate = read_uint32_le(buffer + offset);
    offset += 4;
    metadata->sample_rate = read_uint32_le(buffer + offset);
    offset += 4;
    metadata->channels = read_uint16_le(buffer + offset);
    offset += 2;

    *consumed = offset;
    return true;
}

static bool add_size_checked(size_t* total, size_t value) {
    if (value > (size_t)-1 - *total) return false;
    *total += value;
    return true;
}

static void reset_probe_info(ProbeInfo* info) {
    if (!info) return;
    memset(info, 0, sizeof(ProbeInfo));
    info->audio_format = AudioFormat::NONE;
}

static bool parse_package_data(
    const uint8_t* data,
    size_t size,
    Package* package,
    bool header_only,
    ProbeInfo* info
) {
    if (!data || size < FILE_HEADER_SIZE) return false;
    if (memcmp(data, DMUSICPAK_MAGIC, 4) != 0) return false;

    size_t offset = 4;
    uint32_t version = read_uint32_le(data + offset);
    offset += 4;
    if (version != DMUSICPAK_VERSION) return false;

    uint32_t num_chunks = read_uint32_le(data + offset);
    offset += 4;

    if (info) {
        reset_probe_info(info);
        info->version = version;
        info->num_chunks = num_chunks;
    }

    if (package) {
        package->audio_payload_offset = 0;
        package->has_audio_location = 0;
    }

    for (uint32_t i = 0; i < num_chunks && offset < size; i++) {
        if (size - offset < CHUNK_HEADER_SIZE) return false;

        uint8_t chunk_type = data[offset++];
        uint32_t chunk_size = read_uint32_le(data + offset);
        offset += 4;

        if ((size_t)chunk_size > size - offset) return false;

        size_t chunk_data_offset = offset;

        switch (chunk_type) {
            case CHUNK_METADATA: {
                if (info) info->has_metadata = 1;
                if (!package) break;

                free_metadata(&package->metadata);
                size_t consumed = 0;
                if (!read_metadata_chunk(data + offset, chunk_size, &package->metadata, &consumed) ||
                    consumed != (size_t)chunk_size) {
                    return false;
                }
                package->has_metadata = 1;
                break;
            }

            case CHUNK_LYRICS: {
                if (chunk_size < 4) return false;
                if (info) info->has_lyrics = 1;
                if (!package) break;

                free_lyrics(&package->lyrics);
                package->lyrics.format = (LyricFormat)read_uint32_le(data + offset);
                package->lyrics.size = chunk_size - 4;
                if (!header_only && package->lyrics.size > 0) {
                    package->lyrics.data = (uint8_t*)malloc(package->lyrics.size);
                    if (!package->lyrics.data) return false;
                    memcpy(package->lyrics.data, data + offset + 4, package->lyrics.size);
                } else {
                    package->lyrics.data = NULL;
                }
                package->has_lyrics = 1;
                break;
            }

            case CHUNK_AUDIO: {
                if (chunk_size < 8) return false;

                AudioFormat fmt = (AudioFormat)read_uint32_le(data + offset);
                size_t str_size = 0;
                char* source_filename = NULL;
                if (!read_string(data + offset + 4, chunk_size - 4, &source_filename, &str_size)) return false;

                size_t str_offset = 4 + str_size;
                if (str_offset > chunk_size) {
                    ::free(source_filename);
                    return false;
                }

                size_t audio_size = chunk_size - str_offset;
                size_t audio_offset = chunk_data_offset + str_offset;

                if (info) {
                    info->has_audio = 1;
                    info->audio_format = fmt;
                    info->audio_offset = audio_offset;
                    info->audio_size = audio_size;
                }

                if (!package) {
                    ::free(source_filename);
                    break;
                }

                free_audio(&package->audio);
                package->audio.format = fmt;
                package->audio.source_filename = source_filename;
                package->audio.size = audio_size;
                package->audio_payload_offset = audio_offset;
                package->has_audio_location = 1;
                if (!header_only && audio_size > 0) {
                    package->audio.data = (uint8_t*)malloc(audio_size);
                    if (!package->audio.data) return false;
                    memcpy(package->audio.data, data + offset + str_offset, audio_size);
                } else {
                    package->audio.data = NULL;
                }
                package->has_audio = 1;
                break;
            }

            case CHUNK_COVER:
                if (chunk_size < 12) return false;
                if (info) info->has_cover = 1;
                if (!package) break;

                free_cover(&package->cover);
                package->cover.format = (CoverFormat)read_uint32_le(data + offset);
                package->cover.width = read_uint32_le(data + offset + 4);
                package->cover.height = read_uint32_le(data + offset + 8);
                package->cover.size = chunk_size - 12;
                if (!header_only && package->cover.size > 0) {
                    package->cover.data = (uint8_t*)malloc(package->cover.size);
                    if (!package->cover.data) return false;
                    memcpy(package->cover.data, data + offset + 12, package->cover.size);
                } else {
                    package->cover.data = NULL;
                }
                package->has_cover = 1;
                break;

            default:
                break;
        }

        offset += chunk_size;
    }

    return true;
}

static bool read_file_to_buffer(const char* filename, uint8_t** buffer, size_t* size) {
    if (!filename || !buffer || !size) return false;

    FILE* file = fopen(filename, "rb");
    if (!file) return false;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }
    long file_size = ftell(file);
    if (file_size <= 0) {
        fclose(file);
        return false;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }

    uint8_t* data = (uint8_t*)malloc((size_t)file_size);
    if (!data) {
        fclose(file);
        return false;
    }

    size_t read = fread(data, 1, (size_t)file_size, file);
    fclose(file);
    if (read != (size_t)file_size) {
        ::free(data);
        return false;
    }

    *buffer = data;
    *size = (size_t)file_size;
    return true;
}

Error dmusicpak::save(Package* package, const char* filename) {
    if (!package || !filename) return Error::INVALID_PARAM;

    uint8_t* buffer = NULL;
    size_t size = 0;
    Error result = save_memory(package, &buffer, &size);
    if (result != Error::OK) return result;

    FILE* file = fopen(filename, "wb");
    if (!file) {
        ::free(buffer);
        return Error::FILE_NOT_FOUND;
    }

    size_t written = fwrite(buffer, 1, size, file);
    fclose(file);
    ::free(buffer);

    return (written == size) ? Error::OK : Error::IO;
}

Error dmusicpak::save_memory(Package* package, uint8_t** buffer, size_t* size) {
    if (!package || !buffer || !size) return Error::INVALID_PARAM;
    if (package->has_lyrics && package->lyrics.size > 0 && !package->lyrics.data) return Error::INVALID_PARAM;
    if (package->has_audio && package->audio.size > 0 && !package->audio.data) return Error::INVALID_PARAM;
    if (package->has_cover && package->cover.size > 0 && !package->cover.data) return Error::INVALID_PARAM;

    /* Calculate total size */
    size_t total_size = FILE_HEADER_SIZE;
    uint32_t num_chunks = 0;

    if (package->has_metadata) {
        size_t chunk_size = calculate_metadata_size(&package->metadata);
        if (chunk_size > UINT32_MAX) return Error::INVALID_PARAM;
        if (!add_size_checked(&total_size, CHUNK_HEADER_SIZE + chunk_size)) return Error::INVALID_PARAM;
        num_chunks++;
    }
    if (package->has_lyrics) {
        size_t chunk_size = 4 + package->lyrics.size;
        if (chunk_size > UINT32_MAX) return Error::INVALID_PARAM;
        if (!add_size_checked(&total_size, CHUNK_HEADER_SIZE + chunk_size)) return Error::INVALID_PARAM;
        num_chunks++;
    }
    if (package->has_audio) {
        size_t filename_len = package->audio.source_filename ? strlen(package->audio.source_filename) : 0;
        size_t chunk_size = 4 + 4 + filename_len + package->audio.size;
        if (chunk_size > UINT32_MAX) return Error::INVALID_PARAM;
        if (!add_size_checked(&total_size, CHUNK_HEADER_SIZE + chunk_size)) return Error::INVALID_PARAM;
        num_chunks++;
    }
    if (package->has_cover) {
        size_t chunk_size = 4 + 4 + 4 + package->cover.size;
        if (chunk_size > UINT32_MAX) return Error::INVALID_PARAM;
        if (!add_size_checked(&total_size, CHUNK_HEADER_SIZE + chunk_size)) return Error::INVALID_PARAM;
        num_chunks++;
    }

    /* Allocate buffer */
    *buffer = (uint8_t*)malloc(total_size);
    if (!*buffer) return Error::MEMORY_ALLOC;

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
        uint32_t chunk_size = 4 + (uint32_t)package->lyrics.size;
        write_uint32_le(*buffer + offset, chunk_size);
        offset += 4;
        write_uint32_le(*buffer + offset, (uint32_t)package->lyrics.format);
        offset += 4;
        if (package->lyrics.size > 0) {
            memcpy(*buffer + offset, package->lyrics.data, package->lyrics.size);
            offset += package->lyrics.size;
        }
    }

    /* Write audio chunk */
    if (package->has_audio) {
        (*buffer)[offset++] = CHUNK_AUDIO;
        uint32_t filename_len = package->audio.source_filename ? (uint32_t)strlen(package->audio.source_filename) : 0;
        uint32_t chunk_size = 4 + 4 + filename_len + (uint32_t)package->audio.size;
        write_uint32_le(*buffer + offset, chunk_size);
        offset += 4;
        write_uint32_le(*buffer + offset, (uint32_t)package->audio.format);
        offset += 4;
        offset += write_string(*buffer + offset, package->audio.source_filename);
        if (package->audio.size > 0) {
            memcpy(*buffer + offset, package->audio.data, package->audio.size);
            offset += package->audio.size;
        }
    }

    /* Write cover chunk */
    if (package->has_cover) {
        (*buffer)[offset++] = CHUNK_COVER;
        uint32_t chunk_size = 4 + 4 + 4 + (uint32_t)package->cover.size;
        write_uint32_le(*buffer + offset, chunk_size);
        offset += 4;
        write_uint32_le(*buffer + offset, (uint32_t)package->cover.format);
        offset += 4;
        write_uint32_le(*buffer + offset, package->cover.width);
        offset += 4;
        write_uint32_le(*buffer + offset, package->cover.height);
        offset += 4;
        if (package->cover.size > 0) {
            memcpy(*buffer + offset, package->cover.data, package->cover.size);
            offset += package->cover.size;
        }
    }

    *size = offset;
    return Error::OK;
}

Package* dmusicpak::load(const char* filename) {
    if (!filename) return NULL;

    uint8_t* buffer = NULL;
    size_t size = 0;
    if (!read_file_to_buffer(filename, &buffer, &size)) return NULL;

    Package* package = load_memory(buffer, size);
    ::free(buffer);
    return package;
}

Package* dmusicpak::load_memory(const uint8_t* data, size_t size) {
    Package* package = create();
    if (!package) return NULL;
    if (!parse_package_data(data, size, package, false, NULL)) {
        free(package);
        return NULL;
    }
    return package;
}

Package* dmusicpak::load_header_only(const char* filename) {
    if (!filename) return NULL;

    uint8_t* buffer = NULL;
    size_t size = 0;
    if (!read_file_to_buffer(filename, &buffer, &size)) return NULL;

    Package* package = load_memory_header_only(buffer, size);
    ::free(buffer);
    return package;
}

Package* dmusicpak::load_memory_header_only(const uint8_t* data, size_t size) {
    Package* package = create();
    if (!package) return NULL;
    if (!parse_package_data(data, size, package, true, NULL)) {
        free(package);
        return NULL;
    }
    return package;
}

Error dmusicpak::probe(const char* filename, ProbeInfo* info) {
    if (!filename || !info) return Error::INVALID_PARAM;

    uint8_t* buffer = NULL;
    size_t size = 0;
    if (!read_file_to_buffer(filename, &buffer, &size)) return Error::FILE_NOT_FOUND;

    Error result = probe_memory(buffer, size, info);
    ::free(buffer);
    return result;
}

Error dmusicpak::probe_memory(const uint8_t* data, size_t size, ProbeInfo* info) {
    if (!info || !data || size < FILE_HEADER_SIZE) return Error::INVALID_PARAM;
    if (!parse_package_data(data, size, NULL, true, info)) return Error::INVALID_FORMAT;
    return Error::OK;
}
