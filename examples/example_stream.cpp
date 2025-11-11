/**
 * @file stream_example.cpp
 * @brief Example of streaming audio data from a .dmusicpak file
 */

#include <dmusicpak/dmusicpak.h>
#include <stdio.h>
#include <string.h>

using namespace dmusicpak;

/* Streaming callback function */
static size_t stream_callback(void* buffer, size_t size, size_t nmemb, void* userdata) {
    FILE* output = (FILE*)userdata;
    size_t total = size * nmemb;

    /* Write to output file */
    size_t written = fwrite(buffer, 1, total, output);

    /* Print progress */
    static size_t total_written = 0;
    total_written += written;

    printf("\rStreaming... %zu bytes written", total_written);
    fflush(stdout);

    return written;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.dmusicpak> [output.raw]\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = (argc > 2) ? argv[2] : "output.raw";

    printf("DMusicPak Stream Example\n");
    printf("========================\n\n");
    printf("Library Version: %s\n", version());
    printf("Input file:  %s\n", input_file);
    printf("Output file: %s\n\n", output_file);

    /* Load package */
    Package* package = load(input_file);
    if (!package) {
        fprintf(stderr, "Error: Failed to load package\n");
        return 1;
    }

    printf("[OK] Package loaded successfully\n");

    /* Open output file */
    FILE* output = fopen(output_file, "wb");
    if (!output) {
        fprintf(stderr, "Error: Failed to open output file\n");
        free(package);
        return 1;
    }

    /* Method 1: Stream with callback */
    printf("\nMethod 1: Streaming with callback\n");
    printf("----------------------------------\n");

    Error result = stream_audio(package, stream_callback, output);
    if (result != Error::OK) {
        fprintf(stderr, "\nError streaming audio: %s\n", error_string(result));
        fclose(output);
        free(package);
        return 1;
    }

    printf("\n[OK] Streaming completed successfully\n\n");
    fclose(output);

    /* Method 2: Manual chunk reading */
    printf("Method 2: Manual chunk reading\n");
    printf("------------------------------\n");

    FILE* output2 = fopen("output_chunks.raw", "wb");
    if (!output2) {
        fprintf(stderr, "Error: Failed to open output file for chunks\n");
        free(package);
        return 1;
    }

    const size_t chunk_size = 4096; /* 4KB chunks */
    uint8_t buffer[chunk_size];
    size_t offset = 0;
    size_t total_read = 0;

    while (1) {
        int64_t bytes_read = get_audio_chunk(package, offset, chunk_size, buffer);
        if (bytes_read <= 0) break;

        fwrite(buffer, 1, bytes_read, output2);
        offset += bytes_read;
        total_read += bytes_read;

        printf("\rReading chunks... %zu bytes read", total_read);
        fflush(stdout);
    }

    printf("\n[OK] Chunk reading completed: %zu bytes\n", total_read);
    fclose(output2);

    /* Display audio info */
    Metadata metadata = {0};
    result = get_metadata(package, &metadata);
    if (result == Error::OK) {
        printf("\nAudio Information:\n");
        printf("  Duration:    %.2f seconds\n", metadata.duration_ms / 1000.0);
        printf("  Bitrate:     %u kbps\n", metadata.bitrate);
        printf("  Sample Rate: %u Hz\n", metadata.sample_rate);
        printf("  Channels:    %u\n", metadata.channels);

        free_metadata(&metadata);
    }

    /* Clean up */
    free(package);

    printf("\n[OK] Streaming example completed\n");
    printf("\nOutput files created:\n");
    printf("  - %s (callback method)\n", output_file);
    printf("  - output_chunks.raw (chunk method)\n");

    return 0;
}
