/**
 * @file dmusicpak_c.h
 * @brief DMusicPak C API - C-compatible interface
 * @version 1.0.1
 *
 * This header provides a C-compatible interface for the DMusicPak library.
 * C programs should include this header instead of dmusicpak.h
 */

#ifndef DMUSICPAK_C_H
#define DMUSICPAK_C_H

#include <stdint.h>
#include <stddef.h>

/* Export/Import macros for Windows DLL */
#ifdef _WIN32
    #ifdef DMUSICPAK_EXPORTS
        #define DMUSICPAK_API __declspec(dllexport)
    #else
        #define DMUSICPAK_API __declspec(dllimport)
    #endif
#else
    #define DMUSICPAK_API __attribute__((visibility("default")))
#endif

/* C API - Compatible with C programs */
#ifdef __cplusplus
extern "C" {
#endif

/* C-compatible error codes */
typedef enum {
    DMUSICPAK_ERROR_OK = 0,
    DMUSICPAK_ERROR_INVALID_PARAM = -1,
    DMUSICPAK_ERROR_FILE_NOT_FOUND = -2,
    DMUSICPAK_ERROR_INVALID_FORMAT = -3,
    DMUSICPAK_ERROR_MEMORY_ALLOC = -4,
    DMUSICPAK_ERROR_IO = -5,
    DMUSICPAK_ERROR_NOT_SUPPORTED = -6,
    DMUSICPAK_ERROR_CORRUPTED = -7,
    DMUSICPAK_ERROR_NETWORK = -8
} dmusicpak_error_t;

/* C-compatible lyrics format types */
typedef enum {
    DMUSICPAK_LYRIC_FORMAT_NONE = 0,
    DMUSICPAK_LYRIC_FORMAT_LRC_ESLYRIC = 1,
    DMUSICPAK_LYRIC_FORMAT_LRC_WORD_BY_WORD = 2,
    DMUSICPAK_LYRIC_FORMAT_LRC_LINE_BY_LINE = 3,
    DMUSICPAK_LYRIC_FORMAT_SRT = 4,
    DMUSICPAK_LYRIC_FORMAT_ASS = 5
} dmusicpak_lyric_format_t;

/* C-compatible cover image format types */
typedef enum {
    DMUSICPAK_COVER_FORMAT_NONE = 0,
    DMUSICPAK_COVER_FORMAT_JPEG = 1,
    DMUSICPAK_COVER_FORMAT_PNG = 2,
    DMUSICPAK_COVER_FORMAT_WEBP = 3,
    DMUSICPAK_COVER_FORMAT_BMP = 4
} dmusicpak_cover_format_t;

/* C-compatible audio file format types */
typedef enum {
    DMUSICPAK_AUDIO_FORMAT_NONE = 0,
    DMUSICPAK_AUDIO_FORMAT_MP3 = 1,
    DMUSICPAK_AUDIO_FORMAT_FLAC = 2,
    DMUSICPAK_AUDIO_FORMAT_WAV = 3,
    DMUSICPAK_AUDIO_FORMAT_OGG = 4,
    DMUSICPAK_AUDIO_FORMAT_AAC = 5,
    DMUSICPAK_AUDIO_FORMAT_M4A = 6,
    DMUSICPAK_AUDIO_FORMAT_OPUS = 7,
    DMUSICPAK_AUDIO_FORMAT_WMA = 8,
    DMUSICPAK_AUDIO_FORMAT_APE = 9,
    DMUSICPAK_AUDIO_FORMAT_DSD = 10
} dmusicpak_audio_format_t;

/* C-compatible music metadata structure */
typedef struct {
    char* title;
    char* artist;
    char* album;
    char* genre;
    char* year;
    char* comment;
    uint32_t duration_ms;
    uint32_t bitrate;
    uint32_t sample_rate;
    uint16_t channels;
} dmusicpak_metadata_t;

/* C-compatible lyrics data structure */
typedef struct {
    dmusicpak_lyric_format_t format;
    uint8_t* data;
    size_t size;
} dmusicpak_lyrics_t;

/* C-compatible audio data structure */
typedef struct {
    dmusicpak_audio_format_t format;
    char* source_filename;
    uint8_t* data;
    size_t size;
} dmusicpak_audio_t;

/* C-compatible cover image structure */
typedef struct {
    dmusicpak_cover_format_t format;
    uint8_t* data;
    size_t size;
    uint32_t width;
    uint32_t height;
} dmusicpak_cover_t;

/* Opaque package handle */
typedef void* dmusicpak_package_t;

/* Streaming callback function type */
typedef size_t (*dmusicpak_stream_callback_t)(
    void* buffer,
    size_t size,
    size_t nmemb,
    void* userdata
);

/**
 * @brief Get library version string (C API)
 * @return Version string (e.g., "1.0.0")
 */
DMUSICPAK_API const char* dmusicpak_version(void);

/**
 * @brief Get error message for error code (C API)
 * @param error Error code
 * @return Error message string
 */
DMUSICPAK_API const char* dmusicpak_error_string(dmusicpak_error_t error);

/**
 * @brief Create a new empty package (C API)
 * @return Package handle or NULL on error
 */
DMUSICPAK_API dmusicpak_package_t dmusicpak_create(void);

/**
 * @brief Load package from file (C API)
 * @param filename Path to .dmusicpak file
 * @return Package handle or NULL on error
 */
DMUSICPAK_API dmusicpak_package_t dmusicpak_load(const char* filename);

/**
 * @brief Load package from memory (C API)
 * @param data Pointer to package data
 * @param size Size of data
 * @return Package handle or NULL on error
 */
DMUSICPAK_API dmusicpak_package_t dmusicpak_load_memory(const uint8_t* data, size_t size);

#ifdef DMUSICPAK_ENABLE_NETWORK
/**
 * @brief Load package from URL (HTTP/HTTPS) (C API)
 * @param url URL to load from (must be http:// or https://)
 * @param timeout_ms Timeout in milliseconds (0 for default: 30000ms)
 * @return Package handle or NULL on error
 */
DMUSICPAK_API dmusicpak_package_t dmusicpak_load_url(const char* url, uint32_t timeout_ms);

/**
 * @brief Load package from URL with streaming support (C API)
 * @param url URL to load from (must be http:// or https://)
 * @param timeout_ms Timeout in milliseconds (0 for default: 30000ms)
 * @param chunk_size Streaming chunk size in bytes (0 for default: 64KB)
 * @return Package handle or NULL on error
 */
DMUSICPAK_API dmusicpak_package_t dmusicpak_load_url_stream(const char* url, uint32_t timeout_ms, size_t chunk_size);

/**
 * @brief Get audio chunk from URL using HTTP Range request (C API)
 * @param url URL to load from
 * @param offset Byte offset to start reading from
 * @param size Number of bytes to read
 * @param buffer Output buffer (must be at least 'size' bytes)
 * @param timeout_ms Timeout in milliseconds (0 for default: 30000ms)
 * @return Number of bytes read, or -1 on error
 */
DMUSICPAK_API int64_t dmusicpak_get_audio_chunk_url(
    const char* url,
    size_t offset,
    size_t size,
    uint8_t* buffer,
    uint32_t timeout_ms
);
#endif /* DMUSICPAK_ENABLE_NETWORK */

/**
 * @brief Save package to file (C API)
 * @param package Package handle
 * @param filename Output filename
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_save(dmusicpak_package_t package, const char* filename);

/**
 * @brief Save package to memory buffer (C API)
 * @param package Package handle
 * @param buffer Output buffer (allocated by function)
 * @param size Output size
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_save_memory(dmusicpak_package_t package, uint8_t** buffer, size_t* size);

/**
 * @brief Free package and all associated data (C API)
 * @param package Package handle
 */
DMUSICPAK_API void dmusicpak_free(dmusicpak_package_t package);

/**
 * @brief Set metadata for package (C API)
 * @param package Package handle
 * @param metadata Metadata structure (will be copied)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_set_metadata(dmusicpak_package_t package, const dmusicpak_metadata_t* metadata);

/**
 * @brief Get metadata from package (C API)
 * @param package Package handle
 * @param metadata Output metadata structure (must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_get_metadata(dmusicpak_package_t package, dmusicpak_metadata_t* metadata);

/**
 * @brief Set lyrics for package (C API)
 * @param package Package handle
 * @param lyrics Lyrics structure (data will be copied)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_set_lyrics(dmusicpak_package_t package, const dmusicpak_lyrics_t* lyrics);

/**
 * @brief Get lyrics from package (C API)
 * @param package Package handle
 * @param lyrics Output lyrics structure (data must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_get_lyrics(dmusicpak_package_t package, dmusicpak_lyrics_t* lyrics);

/**
 * @brief Set audio data for package (C API)
 * @param package Package handle
 * @param audio Audio structure (data will be copied)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_set_audio(dmusicpak_package_t package, const dmusicpak_audio_t* audio);

/**
 * @brief Get audio data from package (C API)
 * @param package Package handle
 * @param audio Output audio structure (data must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_get_audio(dmusicpak_package_t package, dmusicpak_audio_t* audio);

/**
 * @brief Set cover image for package (C API)
 * @param package Package handle
 * @param cover Cover structure (data will be copied)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_set_cover(dmusicpak_package_t package, const dmusicpak_cover_t* cover);

/**
 * @brief Get cover image from package (C API)
 * @param package Package handle
 * @param cover Output cover structure (data must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_get_cover(dmusicpak_package_t package, dmusicpak_cover_t* cover);

/**
 * @brief Stream audio data with callback (C API)
 * @param package Package handle
 * @param callback Callback function for streaming data
 * @param userdata User data passed to callback
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_stream_audio(
    dmusicpak_package_t package,
    dmusicpak_stream_callback_t callback,
    void* userdata
);

/**
 * @brief Get audio data chunk for streaming (C API)
 * @param package Package handle
 * @param offset Offset in bytes
 * @param size Size to read
 * @param buffer Output buffer
 * @return Number of bytes read or -1 on error
 */
DMUSICPAK_API int64_t dmusicpak_get_audio_chunk(
    dmusicpak_package_t package,
    size_t offset,
    size_t size,
    uint8_t* buffer
);

/**
 * @brief Free metadata structure (C API)
 * @param metadata Metadata to free
 */
DMUSICPAK_API void dmusicpak_free_metadata(dmusicpak_metadata_t* metadata);

/**
 * @brief Free lyrics structure (C API)
 * @param lyrics Lyrics to free
 */
DMUSICPAK_API void dmusicpak_free_lyrics(dmusicpak_lyrics_t* lyrics);

/**
 * @brief Free audio structure (C API)
 * @param audio Audio to free
 */
DMUSICPAK_API void dmusicpak_free_audio(dmusicpak_audio_t* audio);

/**
 * @brief Free cover structure (C API)
 * @param cover Cover to free
 */
DMUSICPAK_API void dmusicpak_free_cover(dmusicpak_cover_t* cover);

#ifdef __cplusplus
}
#endif

#endif /* DMUSICPAK_C_H */

