/**
* @file internal.h
 * @brief Internal utility functions for DMusicPak library
 * Not part of public API
 */

#ifndef DMUSICPAK_INTERNAL_H
#define DMUSICPAK_INTERNAL_H

#include "dmusicpak/dmusicpak.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* Internal package structure */
    struct dmusicpak_package {
        dmusicpak_metadata_t metadata;
        dmusicpak_lyrics_t lyrics;
        dmusicpak_audio_t audio;
        dmusicpak_cover_t cover;
        int has_metadata;
        int has_lyrics;
        int has_audio;
        int has_cover;
    };

    /* Little-endian integer writing functions */
    void write_uint32_le(uint8_t* buffer, uint32_t value);
    void write_uint16_le(uint8_t* buffer, uint16_t value);

    /* Little-endian integer reading functions */
    uint32_t read_uint32_le(const uint8_t* buffer);
    uint16_t read_uint16_le(const uint8_t* buffer);

#ifdef __cplusplus
}
#endif

#endif /* DMUSICPAK_INTERNAL_H */