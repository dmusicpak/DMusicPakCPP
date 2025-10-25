/**
 * @file write_example.cpp
 * @brief Example of creating and writing a .dmusicpak file
 */

#include <dmusicpak/dmusicpak.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Sample audio data (simulated MP3 header) */
static const uint8_t SAMPLE_AUDIO[] = {
    0xFF, 0xFB, 0x90, 0x00, /* MP3 sync word and header */
    /* ... actual audio data would follow ... */
};

/* Sample LRC lyrics */
static const char* SAMPLE_LYRICS =
    "[ti:Example Song]\n"
    "[ar:Example Artist]\n"
    "[al:Example Album]\n"
    "[00:12.00]First line of lyrics\n"
    "[00:17.50]Second line of lyrics\n"
    "[00:23.00]Third line of lyrics\n";

/* Sample JPEG cover (minimal JPEG header) */
static const uint8_t SAMPLE_COVER[] = {
    0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46,
    0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
    /* ... actual image data would follow ... */
};

int main(int argc, char* argv[]) {
    const char* output_file = (argc > 1) ? argv[1] : "example.dmusicpak";

    printf("DMusicPak Write Example\n");
    printf("=======================\n\n");
    printf("Library Version: %s\n\n", dmusicpak_version());

    /* Create a new package */
    dmusicpak_package_t* package = dmusicpak_create();
    if (!package) {
        fprintf(stderr, "Error: Failed to create package\n");
        return 1;
    }

    /* Set metadata */
    dmusicpak_metadata_t metadata = {0};
    metadata.title = (char*)"Example Song";
    metadata.artist = (char*)"Example Artist";
    metadata.album = (char*)"Example Album";
    metadata.genre = (char*)"Pop";
    metadata.year = (char*)"2025";
    metadata.comment = (char*)"Created with DMusicPak library";
    metadata.duration_ms = 180000; /* 3 minutes */
    metadata.bitrate = 320;         /* 320 kbps */
    metadata.sample_rate = 44100;   /* 44.1 kHz */
    metadata.channels = 2;          /* Stereo */

    dmusicpak_error_t result = dmusicpak_set_metadata(package, &metadata);
    if (result != DMUSICPAK_OK) {
        fprintf(stderr, "Error setting metadata: %s\n", dmusicpak_error_string(result));
        dmusicpak_free(package);
        return 1;
    }
    printf("✓ Metadata set successfully\n");

    /* Set lyrics */
    dmusicpak_lyrics_t lyrics = {0};
    lyrics.format = LYRIC_FORMAT_LRC_LINE_BY_LINE;
    lyrics.data = (uint8_t*)SAMPLE_LYRICS;
    lyrics.size = strlen(SAMPLE_LYRICS);

    result = dmusicpak_set_lyrics(package, &lyrics);
    if (result != DMUSICPAK_OK) {
        fprintf(stderr, "Error setting lyrics: %s\n", dmusicpak_error_string(result));
        dmusicpak_free(package);
        return 1;
    }
    printf("✓ Lyrics set successfully\n");

    /* Set audio data */
    dmusicpak_audio_t audio = {0};
    audio.source_filename = (char*)"example.mp3";
    audio.data = (uint8_t*)SAMPLE_AUDIO;
    audio.size = sizeof(SAMPLE_AUDIO);

    result = dmusicpak_set_audio(package, &audio);
    if (result != DMUSICPAK_OK) {
        fprintf(stderr, "Error setting audio: %s\n", dmusicpak_error_string(result));
        dmusicpak_free(package);
        return 1;
    }
    printf("✓ Audio data set successfully\n");

    /* Set cover image */
    dmusicpak_cover_t cover = {0};
    cover.format = COVER_FORMAT_JPEG;
    cover.data = (uint8_t*)SAMPLE_COVER;
    cover.size = sizeof(SAMPLE_COVER);
    cover.width = 500;
    cover.height = 500;

    result = dmusicpak_set_cover(package, &cover);
    if (result != DMUSICPAK_OK) {
        fprintf(stderr, "Error setting cover: %s\n", dmusicpak_error_string(result));
        dmusicpak_free(package);
        return 1;
    }
    printf("✓ Cover image set successfully\n");

    /* Save to file */
    result = dmusicpak_save(package, output_file);
    if (result != DMUSICPAK_OK) {
        fprintf(stderr, "Error saving package: %s\n", dmusicpak_error_string(result));
        dmusicpak_free(package);
        return 1;
    }

    printf("\n✓ Package saved successfully to: %s\n", output_file);

    /* Clean up */
    dmusicpak_free(package);

    printf("\nPackage contains:\n");
    printf("  - Metadata (Title, Artist, Album, etc.)\n");
    printf("  - Lyrics (LRC format)\n");
    printf("  - Audio data (%zu bytes)\n", sizeof(SAMPLE_AUDIO));
    printf("  - Cover image (JPEG, 500x500)\n");

    return 0;
}