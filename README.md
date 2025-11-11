# DMusicPak

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/dmusicpak/DMusicPakCPP)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

**DMusicPak** is a cross-platform C/C++ library for creating and reading `.dmusicpak` files - a unified music package format that bundles audio, lyrics, metadata, and cover art into a single file.

## 🌟 Features

- **Complete Music Package**: Bundle audio files, lyrics, metadata, and cover art in one file
- **Multiple Lyrics Formats**: Support for LRC (ESLyric, Word-by-Word, Line-by-Line), SRT, and ASS
- **Image Formats**: JPEG, PNG, WebP, and BMP cover art support
- **Streaming Support**: Built-in audio streaming capabilities for network playback
- **Network Streaming**: Optional HTTP/HTTPS support for loading packages from URLs (requires libcurl)
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **Easy-to-Use API**: Simple C API with C++ examples
- **Lightweight**: Minimal dependencies (network support optional)
- **CMake Build System**: Modern build system for easy integration

## 🚀 Installation

### Pre-built Binaries

Download pre-built binaries from the [Releases](https://github.com/yourusername/dmusicpak/releases) page.

### Building from Source

See [QUICKSTART.md](QUICKSTART.md) for detailed build instructions.

**Interactive Build (Recommended):**

The build scripts provide an interactive configuration menu similar to Vue CLI, allowing you to select features and options.

Linux/macOS:
```bash
chmod +x build.sh
./build.sh
```

Windows (PowerShell):
```powershell
.\build.ps1
```

The interactive script will guide you through:
- Build type selection (Release/Debug/MinSizeRel/RelWithDebInfo)
- Network streaming support (requires libcurl)
- Example programs
- Test programs
- Shared/Static library selection
- Installation prefix

**Manual Build:**

If you prefer manual configuration:

Linux/macOS:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_NETWORK=ON
cmake --build .
```

Windows (PowerShell):
```powershell
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022" -DENABLE_NETWORK=ON
cmake --build . --config Release
```

## 🎯 Quick Start

### Creating a Package

```cpp
#include <dmusicpak/dmusicpak.h>

// Create a new package
dmusicpak_package_t* package = dmusicpak_create();

// Set metadata
dmusicpak_metadata_t metadata = {0};
metadata.title = "My Song";
metadata.artist = "My Artist";
metadata.duration_ms = 180000; // 3 minutes
dmusicpak_set_metadata(package, &metadata);

// Add audio data
dmusicpak_audio_t audio = {0};
audio.source_filename = "song.mp3";
audio.data = /* your audio data */;
audio.size = /* data size */;
dmusicpak_set_audio(package, &audio);

// Save to file
dmusicpak_save(package, "output.dmusicpak");

// Clean up
dmusicpak_free(package);
```

### Reading a Package

```cpp
#include <dmusicpak/dmusicpak.h>

// Load package
dmusicpak_package_t* package = dmusicpak_load("song.dmusicpak");

// Get metadata
dmusicpak_metadata_t metadata = {0};
dmusicpak_get_metadata(package, &metadata);
printf("Title: %s\n", metadata.title);
printf("Artist: %s\n", metadata.artist);

// Get audio data
dmusicpak_audio_t audio = {0};
dmusicpak_get_audio(package, &audio);
// Use audio.data and audio.size

// Clean up
dmusicpak_free_metadata(&metadata);
dmusicpak_free_audio(&audio);
dmusicpak_free(package);
```

### Streaming Audio

```cpp
// Callback function for streaming
size_t stream_callback(void* buffer, size_t size, size_t nmemb, void* userdata) {
    // Process audio chunks
    return fwrite(buffer, size, nmemb, (FILE*)userdata);
}

// Stream audio
FILE* output = fopen("output.raw", "wb");
dmusicpak_stream_audio(package, stream_callback, output);
fclose(output);
```

### Network Streaming (Optional)

```cpp
#ifdef DMUSICPAK_ENABLE_NETWORK
// Load package from URL
dmusicpak::Package* package = dmusicpak::load_url("https://example.com/music.dmusicpak", 30000);
if (package) {
    // Use package as normal
    dmusicpak::Metadata metadata = {0};
    dmusicpak::get_metadata(package, &metadata);
    printf("Title: %s\n", metadata.title);
    
    // Get audio chunk directly from URL (HTTP Range request)
    uint8_t buffer[65536];
    int64_t bytes = dmusicpak::get_audio_chunk_url(
        "https://example.com/music.dmusicpak",
        0,           // offset
        65536,       // size
        buffer,      // output buffer
        30000        // timeout (ms)
    );
    
    dmusicpak::free(package);
}
#endif
```

## 📦 File Format

The `.dmusicpak` format is a binary format with the following structure:

```
[File Header]
  - Magic: "DMPK" (4 bytes)
  - Version: uint32 (4 bytes)
  - Number of Chunks: uint32 (4 bytes)

[Chunk 1: Metadata]
  - Type: 0x01
  - Size: uint32
  - Data: Title, Artist, Album, Genre, Year, Comment, Duration, Bitrate, etc.

[Chunk 2: Lyrics]
  - Type: 0x02
  - Size: uint32
  - Format: uint32 (LRC/SRT/ASS)
  - Data: Lyrics text

[Chunk 3: Audio]
  - Type: 0x03
  - Size: uint32
  - Source Filename: string
  - Data: Raw audio data

[Chunk 4: Cover]
  - Type: 0x04
  - Size: uint32
  - Format: uint32 (JPEG/PNG/etc.)
  - Width: uint32
  - Height: uint32
  - Data: Image data
```

For detailed format specification, see [docs/FORMAT_SPEC.md](docs/FORMAT_SPEC.md).

## 📚 API Reference

### Core Functions

```c
// Package management
dmusicpak_package_t* dmusicpak_create(void);
dmusicpak_package_t* dmusicpak_load(const char* filename);
dmusicpak_error_t dmusicpak_save(dmusicpak_package_t* package, const char* filename);
void dmusicpak_free(dmusicpak_package_t* package);

// Metadata operations
dmusicpak_error_t dmusicpak_set_metadata(dmusicpak_package_t* package, const dmusicpak_metadata_t* metadata);
dmusicpak_error_t dmusicpak_get_metadata(dmusicpak_package_t* package, dmusicpak_metadata_t* metadata);

// Lyrics operations
dmusicpak_error_t dmusicpak_set_lyrics(dmusicpak_package_t* package, const dmusicpak_lyrics_t* lyrics);
dmusicpak_error_t dmusicpak_get_lyrics(dmusicpak_package_t* package, dmusicpak_lyrics_t* lyrics);

// Audio operations
dmusicpak_error_t dmusicpak_set_audio(dmusicpak_package_t* package, const dmusicpak_audio_t* audio);
dmusicpak_error_t dmusicpak_get_audio(dmusicpak_package_t* package, dmusicpak_audio_t* audio);

// Cover operations
dmusicpak_error_t dmusicpak_set_cover(dmusicpak_package_t* package, const dmusicpak_cover_t* cover);
dmusicpak_error_t dmusicpak_get_cover(dmusicpak_package_t* package, dmusicpak_cover_t* cover);

// Streaming
dmusicpak_error_t dmusicpak_stream_audio(dmusicpak_package_t* package, 
                                         dmusicpak_stream_callback_t callback, 
                                         void* userdata);
int64_t dmusicpak_get_audio_chunk(dmusicpak_package_t* package, 
                                   size_t offset, 
                                   size_t size, 
                                   uint8_t* buffer);

#ifdef DMUSICPAK_ENABLE_NETWORK
// Network streaming (requires libcurl)
dmusicpak_package_t* dmusicpak_load_url(const char* url, uint32_t timeout_ms);
dmusicpak_package_t* dmusicpak_load_url_stream(const char* url, uint32_t timeout_ms, size_t chunk_size);
int64_t dmusicpak_get_audio_chunk_url(const char* url, size_t offset, size_t size, 
                                       uint8_t* buffer, uint32_t timeout_ms);
#endif
```

### Data Types

```c
typedef enum {
    LYRIC_FORMAT_LRC_ESLYRIC,      // Enhanced LRC
    LYRIC_FORMAT_LRC_WORD_BY_WORD, // Word-by-word timing
    LYRIC_FORMAT_LRC_LINE_BY_LINE, // Line-by-line timing
    LYRIC_FORMAT_SRT,               // SubRip
    LYRIC_FORMAT_ASS                // Advanced SubStation Alpha
} lyric_format_t;

typedef enum {
    COVER_FORMAT_JPEG,
    COVER_FORMAT_PNG,
    COVER_FORMAT_WEBP,
    COVER_FORMAT_BMP
} cover_format_t;

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
```

See [include/dmusicpak/dmusicpak.h](include/dmusicpak/dmusicpak.h) for complete API documentation.

## 💡 Examples

The `examples/` directory contains complete working examples:

- **write_example.cpp**: Creating and saving a package
- **read_example.cpp**: Loading and reading package contents
- **stream_example.cpp**: Streaming audio data

Build and run:
```bash
cd build/bin
./example_write
./example_read example.dmusicpak
./example_stream example.dmusicpak
```

## 🔨 Building from Source

### Requirements

- CMake 3.15 or higher
- C++11 compatible compiler (GCC, Clang, MSVC)
- C99 compatible compiler

### Build Steps

1. Clone the repository:
```bash
git clone https://github.com/dmusicpak/DMusicPakCPP.git
cd dmusicpak
```

2. Run the build script:

**Linux/macOS:**
```bash
chmod +x build.sh
./build.sh Release
```

**Windows:**
```powershell
.\build.ps1 -BuildType Release
```

3. Install (optional):
```bash
cd build
sudo cmake --install .
```

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
cmake --build .
```

### CMake Options

- `BUILD_SHARED_LIBS`: Build shared library (ON/OFF, default: ON)
- `BUILD_EXAMPLES`: Build example programs (ON/OFF, default: ON)
- `BUILD_TESTS`: Build test programs (ON/OFF, default: OFF)
- `CMAKE_INSTALL_PREFIX`: Installation directory

## 🔗 Integration

### CMake Integration

Add to your `CMakeLists.txt`:

```cmake
find_package(DMusicPak REQUIRED)
target_link_libraries(your_target PRIVATE DMusicPak::dmusicpak)
```

### Manual Integration

1. Copy `include/dmusicpak/` to your include path
2. Link against `libdmusicpak.so` (Linux), `libdmusicpak.dylib` (macOS), or `dmusicpak.dll` (Windows)

## 🧪 Testing

Build with tests enabled:
```bash
cmake .. -DBUILD_TESTS=ON
cmake --build .
ctest
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes:

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by various music container formats
- Built with modern C/C++ best practices
- Designed for cross-platform compatibility

## 📞 Support

- 📧 Email: 1067823908@qq.com
- 🐛 Issues: [GitHub Issues](https://github.com/dmusicpak/DMusicPakCPP/issues)
- 📖 Documentation: [Wiki](https://github.com/dmusicpak/DMusicPakCPP/wiki)

## 🗺️ Roadmap

- [ ] Add compression support (zlib, lzma)
- [ ] Implement encryption for protected content
- [ ] Add more lyrics format support
- [ ] Python bindings
- [ ] JavaScript/WASM bindings
- [ ] GUI tools for package creation

---

Made with ❤️ by the [64Hz Games](https://64hz.cn) @ [Type Dream Moon](https://github.com/TypeDreamMoon)