/**
 * @file example_network.cpp
 * @brief Example of loading and streaming .dmusicpak files from network URLs
 * Requires ENABLE_NETWORK=ON and libcurl
 */

#ifdef DMUSICPAK_ENABLE_NETWORK

#include <dmusicpak/dmusicpak.h>
#include <stdio.h>
#include <string.h>

using namespace dmusicpak;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <url> [output_file]\n", argv[0]);
        fprintf(stderr, "Example: %s https://example.com/music.dmusicpak\n", argv[0]);
        return 1;
    }

    const char* url = argv[1];
    const char* output_file = (argc > 2) ? argv[2] : "downloaded.dmusicpak";

    printf("DMusicPak Network Streaming Example\n");
    printf("====================================\n\n");
    printf("Library Version: %s\n", version());
    printf("URL:  %s\n", url);
    printf("Output: %s\n\n", output_file);

    /* Method 1: Load entire file from URL */
    printf("Method 1: Loading entire file from URL\n");
    printf("---------------------------------------\n");
    printf("Downloading...\n");

    Package* package = load_url(url, 30000); /* 30 second timeout (0 for default) */
    if (!package) {
        fprintf(stderr, "Error: Failed to load package from URL\n");
        fprintf(stderr, "Make sure:\n");
        fprintf(stderr, "  1. The URL is accessible\n");
        fprintf(stderr, "  2. The URL points to a valid .dmusicpak file\n");
        fprintf(stderr, "  3. Network connectivity is available\n");
        return 1;
    }

    printf("[OK] Package loaded successfully from URL\n\n");

    /* Display metadata */
    Metadata metadata = {0};
    Error result = get_metadata(package, &metadata);
    if (result == Error::OK) {
        printf("Package Information:\n");
        printf("  Title:       %s\n", metadata.title ? metadata.title : "(none)");
        printf("  Artist:      %s\n", metadata.artist ? metadata.artist : "(none)");
        printf("  Album:       %s\n", metadata.album ? metadata.album : "(none)");
        printf("  Duration:    %.2f seconds\n", metadata.duration_ms / 1000.0);
        printf("  Bitrate:     %u kbps\n", metadata.bitrate);
        printf("  Sample Rate: %u Hz\n", metadata.sample_rate);
        printf("  Channels:    %u\n\n", metadata.channels);
        free_metadata(&metadata);
    }

    /* Save to local file */
    printf("Saving to local file: %s\n", output_file);
    result = save(package, output_file);
    if (result != Error::OK) {
        fprintf(stderr, "Error saving file: %s\n", error_string(result));
        free(package);
        return 1;
    }
    printf("[OK] Saved to %s\n\n", output_file);

    /* Method 2: Stream audio chunks using HTTP Range requests */
    printf("Method 2: Streaming audio chunks using HTTP Range\n");
    printf("---------------------------------------------------\n");

    /* First, we need to know the audio offset in the file */
    /* For this example, we'll use the loaded package */
    Audio audio = {0};
    result = get_audio(package, &audio);
    if (result == Error::OK && audio.size > 0) {
        printf("Audio size: %zu bytes\n", audio.size);
        printf("Streaming first 64KB of audio...\n");

        const size_t chunk_size = 65536; /* 64KB */
        uint8_t* buffer = (uint8_t*)malloc(chunk_size);
        if (buffer) {
            /* Note: In a real scenario, you'd need to calculate the actual
             * byte offset of audio data in the remote file. For this example,
             * we're just demonstrating the API. */
            printf("Note: get_audio_chunk_url requires knowing the byte offset\n");
            printf("      of audio data in the remote file.\n");
            printf("      This is a simplified example.\n\n");

            free(buffer);
        }
        free_audio(&audio);
    }

    /* Clean up */
    free(package);

    printf("[OK] Network streaming example completed\n");
    return 0;
}

#else /* DMUSICPAK_ENABLE_NETWORK */

#include <stdio.h>

int main(int argc, char* argv[]) {
    fprintf(stderr, "Error: Network support is not enabled.\n");
    fprintf(stderr, "Rebuild with: cmake .. -DENABLE_NETWORK=ON\n");
    fprintf(stderr, "Make sure libcurl is installed.\n");
    return 1;
}

#endif /* DMUSICPAK_ENABLE_NETWORK */

