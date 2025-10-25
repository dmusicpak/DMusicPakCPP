# DMusicPak Quick Start Guide

This guide will help you get up and running with DMusicPak in 5 minutes.

## 📋 Prerequisites

Before you begin, ensure you have:

- **C/C++ Compiler**
    - Linux: GCC 7+ or Clang 6+
    - macOS: Xcode Command Line Tools or Homebrew GCC
    - Windows: Visual Studio 2019+ with C++ support
- **CMake** 3.15 or higher
- **Git** (optional, for cloning)

## 🚀 Step 1: Get the Source Code

### Option A: Clone from Git
```bash
git clone https://github.com/dmusicpak/DMusicPakCPP.git
cd dmusicpak
```

### Option B: Download ZIP
Download and extract the ZIP archive, then navigate to the extracted folder.

## 🔧 Step 2: Build the Library

### Linux / macOS

```bash
# Make build script executable
chmod +x build.sh

# Build (Release mode)
./build.sh

# Or specify build type and install prefix
./build.sh Release /usr/local
```

The script will:
1. Check for required tools (CMake, compiler)
2. Create a build directory
3. Configure the project
4. Build the library and examples
5. Offer to run example programs

### Windows

```powershell
# Run PowerShell as Administrator (recommended)
# Navigate to the project directory
cd dmusicpak

# Build
.\build.ps1

# Or specify options
.\build.ps1 -BuildType Release -Generator "Visual Studio 17 2022"
```

### Build Output

After successful build, you'll find:
- **Library**: `build/lib/libdmusicpak.so` (Linux), `libdmusicpak.dylib` (macOS), or `dmusicpak.dll` (Windows)
- **Examples**: `build/bin/write_example`, `read_example`, `stream_example`

## 🎮 Step 3: Run Examples

### Linux / macOS

```bash
cd build/bin

# Create a sample package
./write_example

# Read the package
./read_example example.dmusicpak

# Stream audio data
./stream_example example.dmusicpak
```

### Windows

```powershell
cd build\bin\Release

# Create a sample package
.\write_example.exe

# Read the package
.\read_example.exe example.dmusicpak

# Stream audio data
.\stream_example.exe example.dmusicpak
```

## 📝 Step 4: Write Your First Program

Create a file `my_first_dmusicpak.cpp`:

```cpp
#include <dmusicpak/dmusicpak.h>
#include <stdio.h>
#include <string.h>

int main() {
    // Create a new package
    dmusicpak_package_t* package = dmusicpak_create();
    
    // Set metadata
    dmusicpak_metadata_t metadata = {0};
    metadata.title = (char*)"My First Song";
    metadata.artist = (char*)"Me";
    metadata.duration_ms = 210000;  // 3:30
    metadata.bitrate = 320;
    metadata.sample_rate = 44100;
    metadata.channels = 2;
    
    dmusicpak_set_metadata(package, &metadata);
    
    // Add some dummy audio data
    const char* audio_data = "This would be real audio data";
    dmusicpak_audio_t audio = {0};
    audio.source_filename = (char*)"mysong.mp3";
    audio.data = (uint8_t*)audio_data;
    audio.size = strlen(audio_data);
    
    dmusicpak_set_audio(package, &audio);
    
    // Save the package
    if (dmusicpak_save(package, "myfirst.dmusicpak") == DMUSICPAK_OK) {
        printf("✓ Package created successfully!\n");
    } else {
        printf("✗ Failed to create package\n");
    }
    
    // Clean up
    dmusicpak_free(package);
    
    return 0;
}
```

### Compile and Run

**Linux/macOS:**
```bash
g++ my_first_dmusicpak.cpp -o my_first \
    -I./include \
    -L./build/lib \
    -ldmusicpak

# Run
LD_LIBRARY_PATH=./build/lib ./my_first
```

**Windows:**
```cmd
cl my_first_dmusicpak.cpp /I include /link build\lib\Release\dmusicpak.lib
my_first_dmusicpak.exe
```

## 🔗 Step 5: Integrate into Your Project

### Method 1: CMake (Recommended)

After installing the library (`sudo cmake --install .` in build directory):

```cmake
# In your CMakeLists.txt
find_package(DMusicPak REQUIRED)
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE DMusicPak::dmusicpak)
```

### Method 2: Manual Linking

1. Copy `include/dmusicpak/` to your project's include directory
2. Copy the built library to your project's lib directory
3. Link against the library:
    - Linux: `-ldmusicpak`
    - macOS: `-ldmusicpak`
    - Windows: `dmusicpak.lib`

## 📚 Common Operations

### Create a Complete Package

```cpp
dmusicpak_package_t* package = dmusicpak_create();

// 1. Set Metadata
dmusicpak_metadata_t metadata = {0};
metadata.title = "Song Title";
metadata.artist = "Artist Name";
metadata.album = "Album Name";
metadata.duration_ms = 180000;
dmusicpak_set_metadata(package, &metadata);

// 2. Add Lyrics
const char* lrc_text = "[00:12.00]First line\n[00:15.00]Second line\n";
dmusicpak_lyrics_t lyrics = {0};
lyrics.format = LYRIC_FORMAT_LRC_LINE_BY_LINE;
lyrics.data = (uint8_t*)lrc_text;
lyrics.size = strlen(lrc_text);
dmusicpak_set_lyrics(package, &lyrics);

// 3. Add Audio (load from file)
FILE* audio_file = fopen("song.mp3", "rb");
fseek(audio_file, 0, SEEK_END);
size_t audio_size = ftell(audio_file);
fseek(audio_file, 0, SEEK_SET);
uint8_t* audio_data = (uint8_t*)malloc(audio_size);
fread(audio_data, 1, audio_size, audio_file);
fclose(audio_file);

dmusicpak_audio_t audio = {0};
audio.source_filename = "song.mp3";
audio.data = audio_data;
audio.size = audio_size;
dmusicpak_set_audio(package, &audio);

// 4. Add Cover (load from file)
FILE* cover_file = fopen("cover.jpg", "rb");
fseek(cover_file, 0, SEEK_END);
size_t cover_size = ftell(cover_file);
fseek(cover_file, 0, SEEK_SET);
uint8_t* cover_data = (uint8_t*)malloc(cover_size);
fread(cover_data, 1, cover_size, cover_file);
fclose(cover_file);

dmusicpak_cover_t cover = {0};
cover.format = COVER_FORMAT_JPEG;
cover.data = cover_data;
cover.size = cover_size;
cover.width = 1000;
cover.height = 1000;
dmusicpak_set_cover(package, &cover);

// 5. Save
dmusicpak_save(package, "complete.dmusicpak");

// 6. Clean up
free(audio_data);
free(cover_data);
dmusicpak_free(package);
```

### Read a Package

```cpp
// Load package
dmusicpak_package_t* package = dmusicpak_load("song.dmusicpak");
if (!package) {
    fprintf(stderr, "Failed to load package\n");
    return 1;
}

// Get metadata
dmusicpak_metadata_t metadata = {0};
if (dmusicpak_get_metadata(package, &metadata) == DMUSICPAK_OK) {
    printf("Title: %s\n", metadata.title);
    printf("Artist: %s\n", metadata.artist);
    printf("Duration: %.2f seconds\n", metadata.duration_ms / 1000.0);
    dmusicpak_free_metadata(&metadata);
}

// Get audio
dmusicpak_audio_t audio = {0};
if (dmusicpak_get_audio(package, &audio) == DMUSICPAK_OK) {
    printf("Audio size: %zu bytes\n", audio.size);
    // Save to file
    FILE* out = fopen("extracted.mp3", "wb");
    fwrite(audio.data, 1, audio.size, out);
    fclose(out);
    dmusicpak_free_audio(&audio);
}

// Get lyrics
dmusicpak_lyrics_t lyrics = {0};
if (dmusicpak_get_lyrics(package, &lyrics) == DMUSICPAK_OK) {
    printf("Lyrics:\n%.*s\n", (int)lyrics.size, lyrics.data);
    dmusicpak_free_lyrics(&lyrics);
}

// Clean up
dmusicpak_free(package);
```

### Stream Audio

```cpp
// Custom callback for processing audio chunks
size_t my_callback(void* buffer, size_t size, size_t nmemb, void* userdata) {
    // Process the audio data (e.g., decode, play, etc.)
    size_t total_size = size * nmemb;
    // ... your processing code ...
    return total_size; // Return bytes processed
}

// Stream the audio
dmusicpak_stream_audio(package, my_callback, my_user_data);
```

## 🐛 Troubleshooting

### Build Issues

**CMake not found:**
- Ubuntu/Debian: `sudo apt-get install cmake`
- macOS: `brew install cmake`
- Windows: Download from [cmake.org](https://cmake.org/download/)

**Compiler not found:**
- Ubuntu/Debian: `sudo apt-get install build-essential`
- macOS: `xcode-select --install`
- Windows: Install Visual Studio with C++ workload

**Library not found at runtime (Linux/macOS):**
```bash
# Temporary solution
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/build/lib

# Permanent solution
sudo cmake --install . (in build directory)
```

### Common Errors

**Error: "dmusicpak.h: No such file or directory"**
- Ensure you're including the correct path: `-I/path/to/include`

**Error: "undefined reference to dmusicpak_*"**
- Make sure you're linking the library: `-ldmusicpak` or `dmusicpak.lib`

**Error: "cannot open shared object file"**
- Add library path to `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH` (macOS)
- Or install the library system-wide

## 📖 Next Steps

1. Read the full [README.md](README.md) for detailed documentation
2. Check out more examples in the `examples/` directory
3. Read the [API Reference](README.md#api-reference)
4. See the [File Format Specification](docs/FORMAT_SPEC.md)

## 💬 Getting Help

- Check [GitHub Issues](https://github.com/dmusicpak/DMusicPakCPP/issues)
- Read the [Wiki](https://github.com/dmusicpak/DMusicPakCPP/wiki)
- Contact: 1067823908@qq.com

Happy coding! 🎵
