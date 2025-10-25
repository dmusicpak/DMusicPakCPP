/**
 * @file dmusicpak.h
 * @brief DMusicPak - Cross-platform Music Package Library
 * @version 1.0.1
 *
 * A library for creating and reading .dmusicpak files containing:
 * - Music metadata (title, artist, album, etc.)
 * - Lyrics (LRC, SRT, ASS formats)
 * - Audio data
 * - Cover art (JPEG, PNG)
 * - Streaming support
 */

#ifndef DMUSICPAK_H
#define DMUSICPAK_H

#ifdef __cplusplus
extern "C" {
#endif

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

/* Version information */
#define DMUSICPAK_VERSION_MAJOR 1
#define DMUSICPAK_VERSION_MINOR 0
#define DMUSICPAK_VERSION_PATCH 1

/* Error codes */
typedef enum {
    DMUSICPAK_OK = 0,
    DMUSICPAK_ERROR_INVALID_PARAM = -1,
    DMUSICPAK_ERROR_FILE_NOT_FOUND = -2,
    DMUSICPAK_ERROR_INVALID_FORMAT = -3,
    DMUSICPAK_ERROR_MEMORY_ALLOC = -4,
    DMUSICPAK_ERROR_IO = -5,
    DMUSICPAK_ERROR_NOT_SUPPORTED = -6,
    DMUSICPAK_ERROR_CORRUPTED = -7
} dmusicpak_error_t;

/* Lyrics format types */
typedef enum {
    LYRIC_FORMAT_NONE = 0,
    LYRIC_FORMAT_LRC_ESLYRIC = 1,      /* Enhanced LRC */
    LYRIC_FORMAT_LRC_WORD_BY_WORD = 2, /* Word-by-word timing */
    LYRIC_FORMAT_LRC_LINE_BY_LINE = 3, /* Line-by-line timing */
    LYRIC_FORMAT_SRT = 4,               /* SubRip format */
    LYRIC_FORMAT_ASS = 5                /* Advanced SubStation Alpha */
} lyric_format_t;

/* Cover image format types */
typedef enum {
    COVER_FORMAT_NONE = 0,
    COVER_FORMAT_JPEG = 1,
    COVER_FORMAT_PNG = 2,
    COVER_FORMAT_WEBP = 3,
    COVER_FORMAT_BMP = 4
} cover_format_t;

/* Music metadata structure */
typedef struct {
    char* title;           /* Song title */
    char* artist;          /* Artist name */
    char* album;           /* Album name */
    char* genre;           /* Genre */
    char* year;            /* Release year */
    char* comment;         /* Additional comments */
    uint32_t duration_ms;  /* Duration in milliseconds */
    uint32_t bitrate;      /* Bitrate in kbps */
    uint32_t sample_rate;  /* Sample rate in Hz */
    uint16_t channels;     /* Number of audio channels */
} dmusicpak_metadata_t;

/* Lyrics data structure */
typedef struct {
    lyric_format_t format;
    uint8_t* data;
    size_t size;
} dmusicpak_lyrics_t;

/* Audio data structure */
typedef struct {
    char* source_filename; /* Original filename */
    uint8_t* data;
    size_t size;
} dmusicpak_audio_t;

/* Cover image structure */
typedef struct {
    cover_format_t format;
    uint8_t* data;
    size_t size;
    uint32_t width;
    uint32_t height;
} dmusicpak_cover_t;

/* Main package structure */
typedef struct dmusicpak_package dmusicpak_package_t;

/* Streaming callback function type */
typedef size_t (*dmusicpak_stream_callback_t)(
    void* buffer,
    size_t size,
    size_t nmemb,
    void* userdata
);

/**
 * @brief Get library version string
 * @return Version string (e.g., "1.0.0")
 */
DMUSICPAK_API const char* dmusicpak_version(void);

/**
 * @brief Get error message for error code
 * @param error Error code
 * @return Error message string
 */
DMUSICPAK_API const char* dmusicpak_error_string(dmusicpak_error_t error);

/**
 * @brief Create a new empty package
 * @return Pointer to new package or NULL on error
 */
DMUSICPAK_API dmusicpak_package_t* dmusicpak_create(void);

/**
 * @brief Load package from file
 * @param filename Path to .dmusicpak file
 * @return Pointer to loaded package or NULL on error
 */
DMUSICPAK_API dmusicpak_package_t* dmusicpak_load(const char* filename);

/**
 * @brief Load package from memory
 * @param data Pointer to package data
 * @param size Size of data
 * @return Pointer to loaded package or NULL on error
 */
DMUSICPAK_API dmusicpak_package_t* dmusicpak_load_memory(const uint8_t* data, size_t size);

/**
 * @brief Save package to file
 * @param package Package to save
 * @param filename Output filename
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_save(dmusicpak_package_t* package, const char* filename);

/**
 * @brief Save package to memory buffer
 * @param package Package to save
 * @param buffer Output buffer (allocated by function)
 * @param size Output size
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_save_memory(dmusicpak_package_t* package, uint8_t** buffer, size_t* size);

/**
 * @brief Free package and all associated data
 * @param package Package to free
 */
DMUSICPAK_API void dmusicpak_free(dmusicpak_package_t* package);

/**
 * @brief Set metadata for package
 * @param package Target package
 * @param metadata Metadata structure (will be copied)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_set_metadata(dmusicpak_package_t* package, const dmusicpak_metadata_t* metadata);

/**
 * @brief Get metadata from package
 * @param package Source package
 * @param metadata Output metadata structure (must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_get_metadata(dmusicpak_package_t* package, dmusicpak_metadata_t* metadata);

/**
 * @brief Set lyrics for package
 * @param package Target package
 * @param lyrics Lyrics structure (data will be copied)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_set_lyrics(dmusicpak_package_t* package, const dmusicpak_lyrics_t* lyrics);

/**
 * @brief Get lyrics from package
 * @param package Source package
 * @param lyrics Output lyrics structure (data must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_get_lyrics(dmusicpak_package_t* package, dmusicpak_lyrics_t* lyrics);

/**
 * @brief Set audio data for package
 * @param package Target package
 * @param audio Audio structure (data will be copied)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_set_audio(dmusicpak_package_t* package, const dmusicpak_audio_t* audio);

/**
 * @brief Get audio data from package
 * @param package Source package
 * @param audio Output audio structure (data must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_get_audio(dmusicpak_package_t* package, dmusicpak_audio_t* audio);

/**
 * @brief Set cover image for package
 * @param package Target package
 * @param cover Cover structure (data will be copied)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_set_cover(dmusicpak_package_t* package, const dmusicpak_cover_t* cover);

/**
 * @brief Get cover image from package
 * @param package Source package
 * @param cover Output cover structure (data must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_get_cover(dmusicpak_package_t* package, dmusicpak_cover_t* cover);

/**
 * @brief Stream audio data with callback
 * @param package Source package
 * @param callback Callback function for streaming data
 * @param userdata User data passed to callback
 * @return Error code
 */
DMUSICPAK_API dmusicpak_error_t dmusicpak_stream_audio(
    dmusicpak_package_t* package,
    dmusicpak_stream_callback_t callback,
    void* userdata
);

/**
 * @brief Get audio data chunk for streaming
 * @param package Source package
 * @param offset Offset in bytes
 * @param size Size to read
 * @param buffer Output buffer
 * @return Number of bytes read or -1 on error
 */
DMUSICPAK_API int64_t dmusicpak_get_audio_chunk(
    dmusicpak_package_t* package,
    size_t offset,
    size_t size,
    uint8_t* buffer
);

/**
 * @brief Free metadata structure
 * @param metadata Metadata to free
 */
DMUSICPAK_API void dmusicpak_free_metadata(dmusicpak_metadata_t* metadata);

/**
 * @brief Free lyrics structure
 * @param lyrics Lyrics to free
 */
DMUSICPAK_API void dmusicpak_free_lyrics(dmusicpak_lyrics_t* lyrics);

/**
 * @brief Free audio structure
 * @param audio Audio to free
 */
DMUSICPAK_API void dmusicpak_free_audio(dmusicpak_audio_t* audio);

/**
 * @brief Free cover structure
 * @param cover Cover to free
 */
DMUSICPAK_API void dmusicpak_free_cover(dmusicpak_cover_t* cover);

#ifdef __cplusplus
}
#endif

#endif /* DMUSICPAK_H */