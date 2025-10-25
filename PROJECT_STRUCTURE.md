# DMusicPak Project Structure

This document describes the organization of the DMusicPak project.

## Directory Tree

```
dmusicpak/
├── CMakeLists.txt              # Main CMake configuration
├── README.md                   # Project overview and documentation
├── QUICKSTART.md               # Quick start guide
├── CHANGELOG.md                # Version history and changes
├── .gitignore                  # Git ignore patterns
├── build.sh                    # Linux/macOS build script
├── build.ps1                   # Windows PowerShell build script
│
├── cmake/                      # CMake configuration files
│   └── DMusicPakConfig.cmake.in
│
├── include/                    # Public headers
│   └── dmusicpak/
│       └── dmusicpak.h        # Main public API header
│
├── src/                        # Source implementation
│   ├── dmusicpak.cpp          # Core library implementation
│   ├── io.cpp                 # File I/O operations
│   └── internal.h             # Internal utility functions
│
├── examples/                   # Example programs
│   ├── CMakeLists.txt         # Examples build configuration
│   ├── write_example.cpp      # Create package example
│   ├── read_example.cpp       # Read package example
│   └── stream_example.cpp     # Stream audio example
│
└── docs/                       # Documentation
    └── FORMAT_SPEC.md         # File format specification
```

## File Descriptions

### Root Directory

- **CMakeLists.txt**: Main CMake build configuration that defines the library target, dependencies, and installation rules.

- **README.md**: Comprehensive documentation including:
    - Feature overview
    - Installation instructions
    - Quick start guide
    - API reference
    - Examples

- **QUICKSTART.md**: Step-by-step guide to get started quickly with:
    - Prerequisites
    - Building instructions
    - Running examples
    - Integration guide

- **CHANGELOG.md**: Version history tracking all changes, additions, and bug fixes.

- **LICENSE**: MIT License for open-source distribution.

- **build.sh**: Automated build script for Linux/macOS that:
    - Checks dependencies
    - Configures CMake
    - Builds library and examples
    - Offers to run examples

- **build.ps1**: Automated build script for Windows (PowerShell) with similar functionality.

### include/dmusicpak/

Public API headers that users will include in their projects.

- **dmusicpak.h**: Main C API header containing:
    - All public function declarations
    - Data structure definitions
    - Type definitions (enums, structs)
    - Error codes
    - Platform-specific export macros

### src/

Library implementation files (not installed, internal only).

- **dmusicpak.cpp**: Core library implementation:
    - Package creation and destruction
    - Metadata operations
    - Lyrics operations
    - Audio operations
    - Cover operations
    - Streaming functionality
    - Memory management

- **io.cpp**: File I/O operations:
    - File format reading/writing
    - Binary serialization
    - Chunk-based structure handling
    - Memory buffer operations

- **internal.h**: Internal utility functions:
    - Little-endian integer conversion
    - Helper functions shared between modules

### cmake/

CMake configuration support files.

- **DMusicPakConfig.cmake.in**: Template for generating package configuration file that allows other CMake projects to find and use DMusicPak.

### examples/

Demonstration programs showing library usage.

- **CMakeLists.txt**: Build configuration for examples.

- **write_example.cpp**: Demonstrates:
    - Creating a new package
    - Setting metadata
    - Adding lyrics
    - Adding audio data
    - Adding cover image
    - Saving to file

- **read_example.cpp**: Demonstrates:
    - Loading a package
    - Reading metadata
    - Extracting lyrics
    - Extracting audio
    - Extracting cover image

- **stream_example.cpp**: Demonstrates:
    - Streaming audio with callbacks
    - Manual chunk-based reading
    - Efficient data processing

### docs/

Detailed documentation files.

- **FORMAT_SPEC.md**: Complete file format specification including:
    - Binary structure details
    - Chunk type definitions
    - Data type specifications
    - Parsing guidelines
    - Validation rules
    - Examples with hex dumps

## Build Artifacts

After building, the following directories are created:

```
build/
├── lib/                        # Compiled libraries
│   ├── libdmusicpak.so        # Linux shared library
│   ├── libdmusicpak.dylib     # macOS shared library
│   └── dmusicpak.dll          # Windows DLL
│
└── bin/                        # Example executables
    ├── write_example
    ├── read_example
    └── stream_example
```

## Installation Layout

After installation (`cmake --install .`):

```
/usr/local/                     # Or CMAKE_INSTALL_PREFIX
├── include/
│   └── dmusicpak/
│       └── dmusicpak.h        # Public header
│
├── lib/
│   ├── libdmusicpak.so        # Shared library
│   └── cmake/
│       └── DMusicPak/
│           ├── DMusicPakConfig.cmake
│           ├── DMusicPakConfigVersion.cmake
│           └── DMusicPakTargets.cmake
│
└── bin/
    └── examples/               # Example programs (optional)
        ├── write_example
        ├── read_example
        └── stream_example
```

## Development Workflow

### Adding New Features

1. Update public API in `include/dmusicpak/dmusicpak.h`
2. Implement in `src/dmusicpak.cpp` or `src/io.cpp`
3. Add example usage in `examples/`
4. Update documentation in `README.md` and `docs/`
5. Update `CHANGELOG.md`

### Adding New Chunk Types

1. Define chunk type constant in `src/io.cpp`
2. Add enum value to public header
3. Implement read/write functions in `src/io.cpp`
4. Add API functions in `src/dmusicpak.cpp`
5. Update `docs/FORMAT_SPEC.md`
6. Increment version number

### Testing Changes

1. Build with examples: `BUILD_EXAMPLES=ON`
2. Run example programs
3. Test on all supported platforms
4. Add unit tests in `tests/`

## Code Organization Principles

- **Public API**: Clean C API in header files
- **Implementation**: C++ for convenience, C-compatible ABI
- **Modularity**: Separate concerns (core, I/O, streaming)
- **Portability**: Cross-platform compatible code
- **Documentation**: Inline comments + separate docs
- **Examples**: Practical, runnable demonstrations

## Dependencies

- **Build-time**: CMake 3.15+, C++11 compiler
- **Runtime**: None (zero dependencies)

## Platform Support

- **Linux**: GCC 7+, Clang 6+
- **macOS**: Xcode Command Line Tools
- **Windows**: Visual Studio 2019+

---

For questions about project structure, see [README.md](README.md) or contact the maintainers.