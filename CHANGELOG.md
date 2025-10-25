# Changelog

All notable changes to the DMusicPak project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-25

### Added
- Initial release of DMusicPak library
- Core C API for creating and reading .dmusicpak files
- Support for metadata (title, artist, album, genre, year, comment)
- Support for multiple lyrics formats (LRC ESLyric, Word-by-Word, Line-by-Line, SRT, ASS)
- Support for audio data storage (any format)
- Support for cover art (JPEG, PNG, WebP, BMP)
- Streaming API for audio playback
- Chunk-based audio reading for efficient streaming
- Cross-platform build system using CMake
- Build scripts for Linux/macOS (build.sh) and Windows (build.ps1)
- Comprehensive examples:
    - write_example: Creating packages
    - read_example: Reading packages
    - stream_example: Streaming audio data
- Complete documentation:
    - README.md with full API reference
    - QUICKSTART.md with step-by-step guide
    - FORMAT_SPEC.md with detailed format specification
- MIT License
- .gitignore for common build artifacts

### Features
- Zero dependencies (pure C/C++)
- Little-endian binary format
- Extensible chunk-based structure
- Memory-safe API with proper error handling
- Support for in-memory operations
- Automatic library versioning

### Platforms
- Linux (GCC 7+, Clang 6+)
- macOS (Xcode Command Line Tools)
- Windows (Visual Studio 2019+)

## [Unreleased]

### Planned for 1.1.0
- [ ] Compression support (zlib, lzma)
- [ ] File validation tool
- [ ] File inspection tool
- [ ] Extraction utilities
- [ ] Python bindings
- [ ] Additional examples with real audio files

### Planned for 2.0.0
- [ ] Encryption support
- [ ] Multiple audio tracks
- [ ] Waveform data
- [ ] Checksum/integrity verification
- [ ] JavaScript/WASM bindings
- [ ] GUI tools

---

## Version History

### Version Numbering

We use [Semantic Versioning](https://semver.org/):
- MAJOR version: Incompatible API changes
- MINOR version: Backward-compatible functionality additions
- PATCH version: Backward-compatible bug fixes

### Support Policy

- **Current stable**: Version 1.0.x (Full support)
- **Previous stable**: N/A
- **Legacy**: N/A

---

For detailed commit history, see the [GitHub repository](https://github.com/yourusername/dmusicpak).