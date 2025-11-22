/**
 * @file read_example.cpp
 * @brief Example of reading and extracting data from a .dmusicpak file
 */

#include <dmusicpak/dmusicpak.h>
#include <stdio.h>
#include <string.h>

using namespace dmusicpak;

static const char* lyric_format_string(LyricFormat format) {
    switch (format) {
        case LyricFormat::LRC_ESLYRIC: return "LRC (ESLyric)";
        case LyricFormat::LRC_WORD_BY_WORD: return "LRC (Word-by-Word)";
        case LyricFormat::LRC_LINE_BY_LINE: return "LRC (Line-by-Line)";
        case LyricFormat::SRT: return "SRT";
        case LyricFormat::ASS: return "ASS";
        default: return "Unknown";
    }
}

static const char* cover_format_string(CoverFormat format) {
    switch (format) {
        case CoverFormat::JPEG: return "JPEG";
        case CoverFormat::PNG: return "PNG";
        case CoverFormat::WEBP: return "WebP";
        case CoverFormat::BMP: return "BMP";
        default: return "Unknown";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.dmusicpak>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];

    printf("DMusicPak Read Example\n");
    printf("======================\n\n");
    printf("Library Version: %s\n", version());
    printf("Reading file: %s\n\n", input_file);

    /* Load package */
    Package* package = load(input_file);
    if (!package) {
        fprintf(stderr, "Error: Failed to load package\n");
        return 1;
    }

    printf("[OK] Package loaded successfully\n\n");

    /* Read metadata */
    Metadata metadata = {0};
    Error result = get_metadata(package, &metadata);
    if (result == Error::OK) {
        printf("=== METADATA ===\n");
        if (metadata.title) printf("Title:       %s\n", metadata.title);
        if (metadata.artist) printf("Artist:      %s\n", metadata.artist);
        if (metadata.album) printf("Album:       %s\n", metadata.album);
        if (metadata.genre) printf("Genre:       %s\n", metadata.genre);
        if (metadata.year) printf("Year:        %s\n", metadata.year);
        if (metadata.comment) printf("Comment:     %s\n", metadata.comment);
        printf("Duration:    %u ms (%.2f seconds)\n",
               metadata.duration_ms, metadata.duration_ms / 1000.0);
        printf("Bitrate:     %u kbps\n", metadata.bitrate);
        printf("Sample Rate: %u Hz\n", metadata.sample_rate);
        printf("Channels:    %u\n", metadata.channels);
        printf("\n");

        free_metadata(&metadata);
    } else {
        printf("No metadata found\n\n");
    }

    /* Read lyrics */
    Lyrics lyrics = {};
    result = get_lyrics(package, &lyrics);
    if (result == Error::OK) {
        printf("=== LYRICS ===\n");
        printf("Format: %s\n", lyric_format_string(lyrics.format));
        printf("Size:   %zu bytes\n", lyrics.size);
        printf("\nContent preview (first 200 chars):\n");
        printf("-----------------------------------\n");

        size_t preview_len = lyrics.size < 200 ? lyrics.size : 200;
        for (size_t i = 0; i < preview_len; i++) {
            putchar(lyrics.data[i]);
        }
        if (lyrics.size > 200) {
            printf("\n... (%zu more bytes)", lyrics.size - 200);
        }
        printf("\n\n");

        free_lyrics(&lyrics);
    } else {
        printf("No lyrics found\n\n");
    }

    /* Read audio */
    Audio audio = {AudioFormat::NONE};
    result = get_audio(package, &audio);
    if (result == Error::OK) {
        printf("=== AUDIO ===\n");
        if (audio.source_filename) {
            printf("Source File: %s\n", audio.source_filename);
        }
        printf("Size:        %zu bytes (%.2f MB)\n",
               audio.size, audio.size / (1024.0 * 1024.0));
        printf("First bytes: ");
        size_t preview = audio.size < 16 ? audio.size : 16;
        for (size_t i = 0; i < preview; i++) {
            printf("%02X ", audio.data[i]);
        }
        printf("\n\n");

        free_audio(&audio);
    } else {
        printf("No audio found\n\n");
    }

    /* Read cover */
    Cover cover = {};
    result = get_cover(package, &cover);
    if (result == Error::OK) {
        printf("=== COVER IMAGE ===\n");
        printf("Format:     %s\n", cover_format_string(cover.format));
        printf("Dimensions: %ux%u\n", cover.width, cover.height);
        printf("Size:       %zu bytes (%.2f KB)\n",
               cover.size, cover.size / 1024.0);
        printf("First bytes: ");
        size_t preview = cover.size < 16 ? cover.size : 16;
        for (size_t i = 0; i < preview; i++) {
            printf("%02X ", cover.data[i]);
        }
        printf("\n\n");

        free_cover(&cover);
    } else {
        printf("No cover image found\n\n");
    }

    /* Clean up */
    free(package);

    printf("[OK] Package closed successfully\n");

    return 0;
}
