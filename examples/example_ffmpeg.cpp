/**
 * @file example_ffmpeg.cpp
 * @brief FFmpeg compatibility bridge for .dmusicpak files
 *
 * Usage:
 *   example_ffmpeg <file.dmusicpak>
 *     - Print FFmpeg/FFplay command suggestions
 *
 *   example_ffmpeg <file.dmusicpak> --cat
 *     - Write embedded raw audio payload to stdout
 */

#include <dmusicpak/dmusicpak.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace dmusicpak;

static const char* extension_from_format(AudioFormat format) {
    switch (format) {
        case AudioFormat::MP3: return "mp3";
        case AudioFormat::FLAC: return "flac";
        case AudioFormat::WAV: return "wav";
        case AudioFormat::OGG: return "ogg";
        case AudioFormat::AAC: return "aac";
        case AudioFormat::M4A: return "m4a";
        case AudioFormat::OPUS: return "opus";
        case AudioFormat::WMA: return "wma";
        case AudioFormat::APE: return "ape";
        case AudioFormat::DSD: return "dsf";
        default: return "bin";
    }
}

static int seek_file(FILE* file, size_t offset) {
#ifdef _WIN32
    return _fseeki64(file, (long long)offset, SEEK_SET);
#else
    return fseek(file, (long)offset, SEEK_SET);
#endif
}

static int cat_audio_payload(const char* filename, size_t offset, size_t size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: failed to open input file: %s\n", filename);
        return 1;
    }

    if (seek_file(file, offset) != 0) {
        fprintf(stderr, "Error: failed to seek to audio offset\n");
        fclose(file);
        return 1;
    }

#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    uint8_t buffer[65536];
    size_t remaining = size;
    while (remaining > 0) {
        size_t to_read = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
        size_t read = fread(buffer, 1, to_read, file);
        if (read == 0) {
            fprintf(stderr, "Error: unexpected EOF while reading audio payload\n");
            fclose(file);
            return 1;
        }

        size_t written = fwrite(buffer, 1, read, stdout);
        if (written != read) {
            fprintf(stderr, "Error: failed to write audio payload to stdout\n");
            fclose(file);
            return 1;
        }

        remaining -= read;
    }

    fclose(file);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.dmusicpak> [--cat]\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    const bool cat_mode = (argc >= 3 && strcmp(argv[2], "--cat") == 0);

    ProbeInfo info = {};
    Error result = probe(filename, &info);
    if (result != Error::OK) {
        fprintf(stderr, "Error: failed to probe package: %s\n", error_string(result));
        return 1;
    }

    if (!info.has_audio || info.audio_size == 0) {
        fprintf(stderr, "Error: package has no audio payload\n");
        return 1;
    }

    if (cat_mode) {
        return cat_audio_payload(filename, info.audio_offset, info.audio_size);
    }

    const char* demuxer = audio_format_ffmpeg_demuxer(info.audio_format);
    const char* ext = extension_from_format(info.audio_format);

    printf("DMusicPak FFmpeg Bridge\n");
    printf("=======================\n\n");
    printf("Input:           %s\n", filename);
    printf("Audio Offset:    %llu\n", (unsigned long long)info.audio_offset);
    printf("Audio Size:      %llu bytes\n", (unsigned long long)info.audio_size);
    printf("FFmpeg Demuxer:  %s\n\n", demuxer ? demuxer : "(auto-detect)");

    printf("Windows (PowerShell):\n");
    if (demuxer) {
        printf("  %s \"%s\" --cat | ffplay -f %s -i pipe:0\n", argv[0], filename, demuxer);
        printf("  %s \"%s\" --cat | ffmpeg -f %s -i pipe:0 -c copy output.%s\n", argv[0], filename, demuxer, ext);
    } else {
        printf("  %s \"%s\" --cat | ffplay -i pipe:0\n", argv[0], filename);
        printf("  %s \"%s\" --cat | ffmpeg -i pipe:0 -c copy output.%s\n", argv[0], filename, ext);
    }

    printf("\nLinux/macOS:\n");
    if (demuxer) {
        printf("  ./%s \"%s\" --cat | ffplay -f %s -i pipe:0\n", argv[0], filename, demuxer);
        printf("  ./%s \"%s\" --cat | ffmpeg -f %s -i pipe:0 -c copy output.%s\n", argv[0], filename, demuxer, ext);
    } else {
        printf("  ./%s \"%s\" --cat | ffplay -i pipe:0\n", argv[0], filename);
        printf("  ./%s \"%s\" --cat | ffmpeg -i pipe:0 -c copy output.%s\n", argv[0], filename, ext);
    }

    return 0;
}

