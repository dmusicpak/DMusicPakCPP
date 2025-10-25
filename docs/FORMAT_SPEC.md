# DMusicPak File Format Specification

**Version:** 1.0  
**Status:** Stable  
**Last Updated:** 2025-10-25

## Overview

The `.dmusicpak` format is a binary container format designed to package complete music information including audio data, metadata, lyrics, and cover art in a single file. The format is designed for:

- Easy parsing and streaming
- Extensibility for future features
- Cross-platform compatibility
- Minimal overhead

## File Structure

```
┌─────────────────────────────────────────┐
│           File Header (12 bytes)         │
├─────────────────────────────────────────┤
│         Metadata Chunk (optional)        │
├─────────────────────────────────────────┤
│          Lyrics Chunk (optional)         │
├─────────────────────────────────────────┤
│          Audio Chunk (optional)          │
├─────────────────────────────────────────┤
│          Cover Chunk (optional)          │
└─────────────────────────────────────────┘
```

## Data Types

All multi-byte integers are stored in **little-endian** format.

| Type | Size | Description |
|------|------|-------------|
| uint8 | 1 byte | Unsigned 8-bit integer |
| uint16 | 2 bytes | Unsigned 16-bit integer |
| uint32 | 4 bytes | Unsigned 32-bit integer |
| string | variable | Length-prefixed string (uint32 length + data) |

## File Header

**Size:** 12 bytes

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| 0x00 | char[4] | magic | Magic number: "DMPK" (0x44 0x4D 0x50 0x4B) |
| 0x04 | uint32 | version | Format version (currently 1) |
| 0x08 | uint32 | num_chunks | Number of data chunks following |

### Example

```
44 4D 50 4B  01 00 00 00  04 00 00 00
D  M  P  K   version=1    chunks=4
```

## Chunk Structure

Each chunk consists of:

| Field | Type | Description |
|-------|------|-------------|
| type | uint8 | Chunk type identifier |
| size | uint32 | Size of chunk data in bytes |
| data | byte[size] | Chunk-specific data |

## Chunk Types

### 0x01 - Metadata Chunk

Contains song metadata and information.

**Structure:**

| Field | Type | Description |
|-------|------|-------------|
| title | string | Song title |
| artist | string | Artist/performer name |
| album | string | Album name |
| genre | string | Music genre |
| year | string | Release year |
| comment | string | Additional comments |
| duration_ms | uint32 | Duration in milliseconds |
| bitrate | uint32 | Audio bitrate in kbps |
| sample_rate | uint32 | Sample rate in Hz (e.g., 44100) |
| channels | uint16 | Number of audio channels (1=mono, 2=stereo) |

**String Format:**
```
[uint32: length][bytes: UTF-8 data]
```

Empty strings are represented as length=0 (4 bytes of zeros).

**Example:**
```
Title "My Song" (7 chars):
07 00 00 00 4D 79 20 53 6F 6E 67
length=7    M  y     S  o  n  g
```

### 0x02 - Lyrics Chunk

Contains synchronized or unsynchronized lyrics.

**Structure:**

| Field | Type | Description |
|-------|------|-------------|
| format | uint32 | Lyrics format type |
| data | byte[...] | Lyrics data (format-specific) |

**Format Types:**

| Value | Format | Description |
|-------|--------|-------------|
| 0 | None | No lyrics |
| 1 | LRC (ESLyric) | Enhanced LRC with word timing |
| 2 | LRC (Word-by-Word) | LRC with individual word timestamps |
| 3 | LRC (Line-by-Line) | Standard LRC format |
| 4 | SRT | SubRip subtitle format |
| 5 | ASS | Advanced SubStation Alpha |

**LRC Format Example:**
```
[ti:Song Title]
[ar:Artist Name]
[al:Album Name]
[00:12.00]First line of lyrics
[00:17.20]Second line of lyrics
```

**Data Storage:**
Raw text data in UTF-8 encoding, exactly as shown above.

### 0x03 - Audio Chunk

Contains the actual audio data.

**Structure:**

| Field | Type | Description |
|-------|------|-------------|
| source_filename | string | Original audio filename (e.g., "song.mp3") |
| audio_data | byte[...] | Raw audio file data |

The audio data is stored in its original format (MP3, FLAC, WAV, etc.). No transcoding is performed. The format can be determined from:
1. The source filename extension
2. File magic numbers in the audio data
3. Metadata bitrate/sample_rate fields

**Supported Formats (examples):**
- MP3: Audio data starts with 0xFF 0xFB (sync word)
- FLAC: Audio data starts with "fLaC"
- WAV: Audio data starts with "RIFF"
- OGG: Audio data starts with "OggS"

### 0x04 - Cover Chunk

Contains album artwork or cover image.

**Structure:**

| Field | Type | Description |
|-------|------|-------------|
| format | uint32 | Image format type |
| width | uint32 | Image width in pixels |
| height | uint32 | Image height in pixels |
| image_data | byte[...] | Raw image file data |

**Format Types:**

| Value | Format | Magic Bytes | Description |
|-------|--------|-------------|-------------|
| 0 | None | - | No cover image |
| 1 | JPEG | 0xFF 0xD8 0xFF | JPEG/JPG image |
| 2 | PNG | 0x89 0x50 0x4E 0x47 | PNG image |
| 3 | WebP | "RIFF" ... "WEBP" | WebP image |
| 4 | BMP | 0x42 0x4D | Bitmap image |

The image data is stored in its original format. Width and height are provided for convenience but can also be parsed from the image data.

## Complete File Example

Here's a minimal valid `.dmusicpak` file with all chunks:

```
Offset  Hex Data                              Description
------  ------------------------------------  ---------------------
0x0000  44 4D 50 4B                          Magic "DMPK"
0x0004  01 00 00 00                          Version 1
0x0008  04 00 00 00                          4 chunks

        --- Metadata Chunk ---
0x000C  01                                    Type: Metadata (0x01)
0x000D  XX XX XX XX                          Size
0x0011  [title string]
        [artist string]
        [album string]
        [genre string]
        [year string]
        [comment string]
        [duration_ms: 4 bytes]
        [bitrate: 4 bytes]
        [sample_rate: 4 bytes]
        [channels: 2 bytes]

        --- Lyrics Chunk ---
        02                                    Type: Lyrics (0x02)
        XX XX XX XX                          Size
        03 00 00 00                          Format: LRC Line-by-Line
        [lyrics text data]

        --- Audio Chunk ---
        03                                    Type: Audio (0x03)
        XX XX XX XX                          Size
        [source_filename string]
        [audio data bytes]

        --- Cover Chunk ---
        04                                    Type: Cover (0x04)
        XX XX XX XX                          Size
        01 00 00 00                          Format: JPEG
        E8 03 00 00                          Width: 1000
        E8 03 00 00                          Height: 1000
        [image data bytes]
```

## Parsing Guidelines

### Reading a File

1. Read and verify magic number ("DMPK")
2. Read version number (must be 1 for this spec)
3. Read number of chunks
4. For each chunk:
   - Read chunk type (1 byte)
   - Read chunk size (4 bytes)
   - Read chunk data (size bytes)
   - Parse chunk data based on type

### Writing a File

1. Write file header (magic, version, chunk count)
2. For each data component (metadata, lyrics, audio, cover):
   - Calculate chunk size
   - Write chunk type
   - Write chunk size
   - Write chunk data

### Streaming Considerations

For streaming playback:
1. Read file header and all chunk headers first
2. Locate audio chunk offset and size
3. Stream audio data in chunks (recommended: 8KB blocks)
4. Metadata and lyrics can be loaded separately

## Validation Rules

A valid `.dmusicpak` file must:

1. Start with magic bytes "DMPK" (0x44 0x4D 0x50 0x4B)
2. Have version = 1
3. Have num_chunks matching actual chunk count
4. Each chunk must have:
   - Valid type (0x01-0x04)
   - Size matching actual data
5. All string lengths must match actual string data
6. Audio chunk should contain valid audio data
7. Cover chunk should contain valid image data

## Error Handling

Readers should handle:

- **Invalid magic**: Reject file immediately
- **Unsupported version**: Reject or attempt best-effort parsing
- **Missing chunks**: All chunks are optional except file header
- **Corrupted data**: Validate chunk sizes and data integrity
- **Unknown chunk types**: Skip unknown chunks gracefully

## Future Extensions

### Version 2.0 (Planned)

Potential additions:
- Compression chunk (0x05)
- Encryption metadata (0x06)
- Multiple audio tracks (0x07)
- Timed comments/annotations (0x08)
- Waveform data (0x09)
- Checksum/hash (0x0A)

### Backward Compatibility

Future versions must:
1. Maintain the same file header structure
2. Allow parsers to skip unknown chunk types
3. Clearly version incompatible changes

## Reference Implementation

The official reference implementation is available at:
https://github.com/dmusicpak/DMusicPakCPP

## Appendix A: Complete Example

A complete example with actual bytes can be found in the `examples/` directory:
- `write_example.cpp` - Creates a complete package
- `read_example.cpp` - Reads and validates a package

## Appendix B: Tools

### Validation Tool
```bash
# Validate a .dmusicpak file
dmusicpak-validate file.dmusicpak
```

### Inspection Tool
```bash
# Inspect file structure
dmusicpak-inspect file.dmusicpak
```

### Conversion Tools
```bash
# Extract audio
dmusicpak-extract -a file.dmusicpak -o audio.mp3

# Extract cover
dmusicpak-extract -c file.dmusicpak -o cover.jpg

# Extract lyrics
dmusicpak-extract -l file.dmusicpak -o lyrics.lrc
```

## Contact

For questions about this specification:
- GitHub Issues: https://github.com/dmusicpak/DMusicPakCPP/issues
- Email: 1067823908@qq.com

---

**Document Version:** 1.0  
**Copyright:** (c) 2025 DMusicPak Project  
**License:** CC BY-SA 4.0
