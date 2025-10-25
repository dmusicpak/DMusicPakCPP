#!/bin/bash
# DMusicPak Build Script for Linux/macOS
# Automatically builds the library and examples

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE=${1:-Release}
BUILD_DIR="build"
INSTALL_PREFIX=${2:-/usr/local}

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  DMusicPak Build Script${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed${NC}"
    echo "Please install CMake first:"
    echo "  Ubuntu/Debian: sudo apt-get install cmake"
    echo "  macOS:         brew install cmake"
    exit 1
fi

# Check CMake version
CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
echo -e "${GREEN}✓${NC} CMake version: $CMAKE_VERSION"

# Detect compiler
if command -v g++ &> /dev/null; then
    COMPILER="g++"
    COMPILER_VERSION=$(g++ --version | head -n1)
elif command -v clang++ &> /dev/null; then
    COMPILER="clang++"
    COMPILER_VERSION=$(clang++ --version | head -n1)
else
    echo -e "${RED}Error: No C++ compiler found${NC}"
    exit 1
fi
echo -e "${GREEN}✓${NC} Compiler: $COMPILER_VERSION"

# Detect OS
OS=$(uname -s)
echo -e "${GREEN}✓${NC} Operating System: $OS"

echo ""
echo -e "${YELLOW}Configuration:${NC}"
echo "  Build Type:      $BUILD_TYPE"
echo "  Build Directory: $BUILD_DIR"
echo "  Install Prefix:  $INSTALL_PREFIX"
echo ""

# Create build directory
echo -e "${BLUE}Creating build directory...${NC}"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure with CMake
echo -e "${BLUE}Configuring project...${NC}"
cd "$BUILD_DIR"
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_TESTS=OFF

# Build
echo ""
echo -e "${BLUE}Building project...${NC}"
cmake --build . --config "$BUILD_TYPE" -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Check build success
if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  Build Successful!${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    echo "Library location:"
    if [ "$OS" == "Darwin" ]; then
        echo "  $BUILD_DIR/lib/libdmusicpak.dylib"
    else
        echo "  $BUILD_DIR/lib/libdmusicpak.so"
    fi
    echo ""
    echo "Examples built:"
    echo "  $BUILD_DIR/bin/write_example"
    echo "  $BUILD_DIR/bin/read_example"
    echo "  $BUILD_DIR/bin/stream_example"
    echo ""
    echo "To run examples:"
    echo "  cd $BUILD_DIR/bin"
    echo "  ./write_example"
    echo "  ./read_example example.dmusicpak"
    echo "  ./stream_example example.dmusicpak"
    echo ""
    echo "To install (requires sudo):"
    echo "  sudo cmake --install ."
    echo ""
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

# Offer to run tests
echo -e "${YELLOW}Run write example? (y/n)${NC}"
read -r response
if [[ "$response" =~ ^[Yy]$ ]]; then
    cd bin
    ./write_example
    echo ""
    echo -e "${GREEN}✓ Example package created: example.dmusicpak${NC}"

    echo -e "${YELLOW}Run read example? (y/n)${NC}"
    read -r response2
    if [[ "$response2" =~ ^[Yy]$ ]]; then
        ./read_example example.dmusicpak
    fi
fi

echo ""
echo -e "${GREEN}Build script completed successfully!${NC}"