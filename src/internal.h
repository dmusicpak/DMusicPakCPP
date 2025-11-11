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
namespace dmusicpak {

    /* Internal package structure */
    struct Package {
        Metadata metadata;
        Lyrics lyrics;
        Audio audio;
        Cover cover;
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

} // namespace dmusicpak
#endif

#endif /* DMUSICPAK_INTERNAL_H */
