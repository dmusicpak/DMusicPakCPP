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
#define DMUSICPAK_VERSION_MINOR 1
#define DMUSICPAK_VERSION_PATCH 0

#ifdef __cplusplus
namespace dmusicpak {

/* Error codes */
enum class Error {
    OK = 0,
    INVALID_PARAM = -1,
    FILE_NOT_FOUND = -2,
    INVALID_FORMAT = -3,
    MEMORY_ALLOC = -4,
    IO = -5,
    NOT_SUPPORTED = -6,
    CORRUPTED = -7,
    NETWORK = -8              /* Network error (when network support enabled) */
};

/* Lyrics format types */
enum class LyricFormat {
    NONE = 0,
    LRC_ESLYRIC = 1,      /* Enhanced LRC */
    LRC_WORD_BY_WORD = 2, /* Word-by-word timing */
    LRC_LINE_BY_LINE = 3, /* Line-by-line timing */
    SRT = 4,               /* SubRip format */
    ASS = 5                /* Advanced SubStation Alpha */
};

/* Cover image format types */
enum class CoverFormat {
    NONE = 0,
    JPEG = 1,
    PNG = 2,
    WEBP = 3,
    BMP = 4
};

/* Audio file format types */
enum class AudioFormat {
    NONE = 0,
    MP3 = 1,        /* MPEG Audio Layer III */
    FLAC = 2,       /* Free Lossless Audio Codec */
    WAV = 3,        /* Waveform Audio File Format */
    OGG = 4,        /* Ogg Vorbis */
    AAC = 5,        /* Advanced Audio Coding */
    M4A = 6,        /* MPEG-4 Audio */
    OPUS = 7,       /* Opus audio codec */
    WMA = 8,        /* Windows Media Audio */
    APE = 9,        /* Monkey's Audio */
    DSD = 10        /* Direct Stream Digital */
};

/* Music metadata structure */
struct Metadata {
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
};

/* Lyrics data structure */
struct Lyrics {
    LyricFormat format;
    uint8_t* data;
    size_t size;
};

/* Audio data structure */
struct Audio {
    AudioFormat format;     /* Audio file format */
    char* source_filename; /* Original filename */
    uint8_t* data;
    size_t size;
};

/* Cover image structure */
struct Cover {
    CoverFormat format;
    uint8_t* data;
    size_t size;
    uint32_t width;
    uint32_t height;
};

/* Main package structure */
struct Package;

/* Streaming callback function type */
using StreamCallback = size_t (*)(
    void* buffer,
    size_t size,
    size_t nmemb,
    void* userdata
);

/**
 * @brief Get library version string
 * @return Version string (e.g., "1.0.0")
 */
DMUSICPAK_API const char* version();

/**
 * @brief Get error message for error code
 * @param error Error code
 * @return Error message string
 */
DMUSICPAK_API const char* error_string(Error error);

/**
 * @brief Create a new empty package
 * @return Pointer to new package or NULL on error
 */
DMUSICPAK_API Package* create();

/**
 * @brief Load package from file
 * @param filename Path to .dmusicpak file
 * @return Pointer to loaded package or NULL on error
 */
DMUSICPAK_API Package* load(const char* filename);

/**
 * @brief Load package from memory
 * @param data Pointer to package data
 * @param size Size of data
 * @return Pointer to loaded package or NULL on error
 */
DMUSICPAK_API Package* load_memory(const uint8_t* data, size_t size);

#ifdef DMUSICPAK_ENABLE_NETWORK
/**
 * @brief Load package from URL (HTTP/HTTPS)
 * Downloads entire file into memory before parsing
 * @param url URL to load from (must be http:// or https://)
 * @param timeout_ms Timeout in milliseconds (0 for default: 30000ms)
 * @return Pointer to loaded package or NULL on error
 */
DMUSICPAK_API Package* load_url(const char* url, uint32_t timeout_ms);

/**
 * @brief Load package from URL with streaming support
 * Streams data and parses incrementally (more efficient for large files)
 * @param url URL to load from (must be http:// or https://)
 * @param timeout_ms Timeout in milliseconds (0 for default: 30000ms)
 * @param chunk_size Streaming chunk size in bytes (0 for default: 64KB)
 * @return Pointer to loaded package or NULL on error
 */
DMUSICPAK_API Package* load_url_stream(const char* url, uint32_t timeout_ms, size_t chunk_size);

/**
 * @brief Get audio chunk from URL using HTTP Range request
 * Useful for streaming audio without downloading entire file
 * @param url URL to load from
 * @param offset Byte offset to start reading from
 * @param size Number of bytes to read
 * @param buffer Output buffer (must be at least 'size' bytes)
 * @param timeout_ms Timeout in milliseconds (0 for default: 30000ms)
 * @return Number of bytes read, or -1 on error
 */
DMUSICPAK_API int64_t get_audio_chunk_url(
    const char* url,
    size_t offset,
    size_t size,
    uint8_t* buffer,
    uint32_t timeout_ms
);
#endif /* DMUSICPAK_ENABLE_NETWORK */

/**
 * @brief Save package to file
 * @param package Package to save
 * @param filename Output filename
 * @return Error code
 */
DMUSICPAK_API Error save(Package* package, const char* filename);

/**
 * @brief Save package to memory buffer
 * @param package Package to save
 * @param buffer Output buffer (allocated by function)
 * @param size Output size
 * @return Error code
 */
DMUSICPAK_API Error save_memory(Package* package, uint8_t** buffer, size_t* size);

/**
 * @brief Free package and all associated data
 * @param package Package to free
 */
DMUSICPAK_API void free(Package* package);

/**
 * @brief Set metadata for package
 * @param package Target package
 * @param metadata Metadata structure (will be copied)
 * @return Error code
 */
DMUSICPAK_API Error set_metadata(Package* package, const Metadata* metadata);

/**
 * @brief Get metadata from package
 * @param package Source package
 * @param metadata Output metadata structure (must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API Error get_metadata(Package* package, Metadata* metadata);

/**
 * @brief Set lyrics for package
 * @param package Target package
 * @param lyrics Lyrics structure (data will be copied)
 * @return Error code
 */
DMUSICPAK_API Error set_lyrics(Package* package, const Lyrics* lyrics);

/**
 * @brief Get lyrics from package
 * @param package Source package
 * @param lyrics Output lyrics structure (data must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API Error get_lyrics(Package* package, Lyrics* lyrics);

/**
 * @brief Set audio data for package
 * @param package Target package
 * @param audio Audio structure (data will be copied)
 * @return Error code
 */
DMUSICPAK_API Error set_audio(Package* package, const Audio* audio);

/**
 * @brief Get audio data from package
 * @param package Source package
 * @param audio Output audio structure (data must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API Error get_audio(Package* package, Audio* audio);

/**
 * @brief Set cover image for package
 * @param package Target package
 * @param cover Cover structure (data will be copied)
 * @return Error code
 */
DMUSICPAK_API Error set_cover(Package* package, const Cover* cover);

/**
 * @brief Get cover image from package
 * @param package Source package
 * @param cover Output cover structure (data must be freed by caller)
 * @return Error code
 */
DMUSICPAK_API Error get_cover(Package* package, Cover* cover);

/**
 * @brief Stream audio data with callback
 * @param package Source package
 * @param callback Callback function for streaming data
 * @param userdata User data passed to callback
 * @return Error code
 */
DMUSICPAK_API Error stream_audio(
    Package* package,
    StreamCallback callback,
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
DMUSICPAK_API int64_t get_audio_chunk(
    Package* package,
    size_t offset,
    size_t size,
    uint8_t* buffer
);

/**
 * @brief Free metadata structure
 * @param metadata Metadata to free
 */
DMUSICPAK_API void free_metadata(Metadata* metadata);

/**
 * @brief Free lyrics structure
 * @param lyrics Lyrics to free
 */
DMUSICPAK_API void free_lyrics(Lyrics* lyrics);

/**
 * @brief Free audio structure
 * @param audio Audio to free
 */
DMUSICPAK_API void free_audio(Audio* audio);

/**
 * @brief Free cover structure
 * @param cover Cover to free
 */
DMUSICPAK_API void free_cover(Cover* cover);

} // namespace dmusicpak

#endif /* __cplusplus */

#endif /* DMUSICPAK_H */
