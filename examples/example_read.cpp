/**
 * @file read_example.cpp
 * @brief Example of reading and extracting data from a .dmusicpak file
 */

#include <dmusicpak/dmusicpak.h>
#include <stdio.h>
#include <string.h>

static const char* lyric_format_string(lyric_format_t format) {
    switch (format) {
        case LYRIC_FORMAT_LRC_ESLYRIC: return "LRC (ESLyric)";
        case LYRIC_FORMAT_LRC_WORD_BY_WORD: return "LRC (Word-by-Word)";
        case LYRIC_FORMAT_LRC_LINE_BY_LINE: return "LRC (Line-by-Line)";
        case LYRIC_FORMAT_SRT: return "SRT";
        case LYRIC_FORMAT_ASS: return "ASS";
        default: return "Unknown";
    }
}

static const char* cover_format_string(cover_format_t format) {
    switch (format) {
        case COVER_FORMAT_JPEG: return "JPEG";
        case COVER_FORMAT_PNG: return "PNG";
        case COVER_FORMAT_WEBP: return "WebP";
        case COVER_FORMAT_BMP: return "BMP";
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
    printf("Library Version: %s\n", dmusicpak_version());
    printf("Reading file: %s\n\n", input_file);

    /* Load package */
    dmusicpak_package_t* package = dmusicpak_load(input_file);
    if (!package) {
        fprintf(stderr, "Error: Failed to load package\n");
        return 1;
    }

    printf("✓ Package loaded successfully\n\n");

    /* Read metadata */
    dmusicpak_metadata_t metadata = {0};
    dmusicpak_error_t result = dmusicpak_get_metadata(package, &metadata);
    if (result == DMUSICPAK_OK) {
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

        dmusicpak_free_metadata(&metadata);
    } else {
        printf("No metadata found\n\n");
    }

    /* Read lyrics */
    dmusicpak_lyrics_t lyrics = {0};
    result = dmusicpak_get_lyrics(package, &lyrics);
    if (result == DMUSICPAK_OK) {
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

        dmusicpak_free_lyrics(&lyrics);
    } else {
        printf("No lyrics found\n\n");
    }

    /* Read audio */
    dmusicpak_audio_t audio = {0};
    result = dmusicpak_get_audio(package, &audio);
    if (result == DMUSICPAK_OK) {
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

        dmusicpak_free_audio(&audio);
    } else {
        printf("No audio found\n\n");
    }

    /* Read cover */
    dmusicpak_cover_t cover = {0};
    result = dmusicpak_get_cover(package, &cover);
    if (result == DMUSICPAK_OK) {
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

        dmusicpak_free_cover(&cover);
    } else {
        printf("No cover image found\n\n");
    }

    /* Clean up */
    dmusicpak_free(package);

    printf("✓ Package closed successfully\n");

    return 0;
}