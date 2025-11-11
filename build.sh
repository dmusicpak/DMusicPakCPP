#!/bin/bash
# DMusicPak Interactive Build Script for Linux/macOS
# Similar to Vue CLI - interactive feature selection

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Function to print header
print_header() {
    echo -e "${CYAN}"
    echo "╔════════════════════════════════════════╗"
    echo "║     DMusicPak Build Configuration     ║"
    echo "╚════════════════════════════════════════╝"
    echo -e "${NC}"
}

# Function to print section
print_section() {
    echo -e "${BLUE}${BOLD}▶ $1${NC}"
}

# Function to print option
print_option() {
    echo -e "  ${YELLOW}$1${NC} $2"
}

# Function to print selected
print_selected() {
    echo -e "  ${GREEN}✓${NC} $1"
}

# Function to ask yes/no with default
ask_yn() {
    local prompt="$1"
    local default="$2"
    local result
    
    if [ "$default" = "y" ]; then
        prompt="$prompt ${GREEN}[Y/n]${NC}: "
    else
        prompt="$prompt ${GREEN}[y/N]${NC}: "
    fi
    
    while true; do
        read -p "$(echo -e "$prompt")" result
        result=${result:-$default}
        case $result in
            [Yy]* ) return 0;;
            [Nn]* ) return 1;;
            * ) echo -e "${RED}Please answer yes or no.${NC}";;
        esac
    done
}

# Function to select from menu
select_option() {
    local prompt="$1"
    shift
    local options=("$@")
    local selected=0
    local key
    
    while true; do
        clear
        print_header
        echo -e "${BOLD}$prompt${NC}"
        echo ""
        
        for i in "${!options[@]}"; do
            if [ $i -eq $selected ]; then
                echo -e "  ${GREEN}${BOLD}▶${NC} ${GREEN}${BOLD}${options[$i]}${NC}"
            else
                echo -e "    ${options[$i]}"
            fi
        done
        
        echo ""
        echo -e "${CYAN}Use ↑↓ to navigate, Enter to select${NC}"
        
        read -rsn1 key
        
        case $key in
            $'\x1b')
                read -rsn1 -t 0.1 key
                if [ "$key" = "[" ]; then
                    read -rsn1 -t 0.1 key
                    case $key in
                        A) # Up arrow
                            selected=$((selected - 1))
                            if [ $selected -lt 0 ]; then
                                selected=$((${#options[@]} - 1))
                            fi
                            ;;
                        B) # Down arrow
                            selected=$((selected + 1))
                            if [ $selected -ge ${#options[@]} ]; then
                                selected=0
                            fi
                            ;;
                    esac
                fi
                ;;
            "")
                echo $selected
                return
                ;;
        esac
    done
}

# Start interactive configuration
clear
print_header

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed${NC}"
    echo "Please install CMake first:"
    echo "  Ubuntu/Debian: sudo apt-get install cmake"
    echo "  macOS:         brew install cmake"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
echo -e "${GREEN}✓${NC} CMake version: $CMAKE_VERSION"

# Detect compiler
if command -v g++ &> /dev/null; then
    COMPILER_VERSION=$(g++ --version | head -n1)
elif command -v clang++ &> /dev/null; then
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

# Configuration options
print_section "Build Configuration"

# Build Type
BUILD_TYPE=$(select_option "Select Build Type:" "Release" "Debug" "MinSizeRel" "RelWithDebInfo")
case $BUILD_TYPE in
    0) BUILD_TYPE="Release" ;;
    1) BUILD_TYPE="Debug" ;;
    2) BUILD_TYPE="MinSizeRel" ;;
    3) BUILD_TYPE="RelWithDebInfo" ;;
esac
print_selected "Build Type: $BUILD_TYPE"
echo ""

# Network Support
print_section "Feature Selection"
if ask_yn "Enable Network Streaming Support (requires libcurl)?" "n"; then
    ENABLE_NETWORK="ON"
    print_selected "Network Streaming: Enabled"
    
    # Check for libcurl
    if ! pkg-config --exists libcurl 2>/dev/null && \
       ! find /usr/lib* /usr/local/lib* -name "*curl*" -type f 2>/dev/null | grep -q .; then
        echo -e "${YELLOW}⚠ Warning: libcurl may not be installed${NC}"
        echo "  Install with:"
        echo "    Ubuntu/Debian: sudo apt-get install libcurl4-openssl-dev"
        echo "    macOS:         brew install curl"
        if ! ask_yn "Continue anyway?" "y"; then
            exit 1
        fi
    fi
else
    ENABLE_NETWORK="OFF"
    print_selected "Network Streaming: Disabled"
fi
echo ""

# Build Examples
if ask_yn "Build Example Programs?" "y"; then
    BUILD_EXAMPLES="ON"
    print_selected "Examples: Enabled"
else
    BUILD_EXAMPLES="OFF"
    print_selected "Examples: Disabled"
fi
echo ""

# Build Tests
if ask_yn "Build Test Programs?" "n"; then
    BUILD_TESTS="ON"
    print_selected "Tests: Enabled"
else
    BUILD_TESTS="OFF"
    print_selected "Tests: Disabled"
fi
echo ""

# Build Shared Libraries
if ask_yn "Build Shared Libraries (DLL/SO)?" "y"; then
    BUILD_SHARED_LIBS="ON"
    print_selected "Shared Libraries: Enabled"
else
    BUILD_SHARED_LIBS="OFF"
    print_selected "Shared Libraries: Disabled (Static)"
fi
echo ""

# Install Prefix (optional)
print_section "Installation (Optional)"
read -p "$(echo -e "${YELLOW}Install Prefix${NC} ${GREEN}[default: /usr/local]${NC}: ")" INSTALL_PREFIX
INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}
print_selected "Install Prefix: $INSTALL_PREFIX"
echo ""

# Summary
print_section "Configuration Summary"
echo -e "  ${CYAN}Build Type:${NC}        $BUILD_TYPE"
echo -e "  ${CYAN}Network Support:${NC}    $ENABLE_NETWORK"
echo -e "  ${CYAN}Build Examples:${NC}    $BUILD_EXAMPLES"
echo -e "  ${CYAN}Build Tests:${NC}       $BUILD_TESTS"
echo -e "  ${CYAN}Shared Libraries:${NC}   $BUILD_SHARED_LIBS"
echo -e "  ${CYAN}Install Prefix:${NC}    $INSTALL_PREFIX"
echo ""

if ! ask_yn "Proceed with build?" "y"; then
    echo -e "${YELLOW}Build cancelled.${NC}"
    exit 0
fi

# Build directory
BUILD_DIR="build"

# Create build directory
echo ""
print_section "Preparing Build"
echo -e "${BLUE}Creating build directory...${NC}"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure with CMake
echo -e "${BLUE}Configuring project with CMake...${NC}"
cd "$BUILD_DIR"

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
    -DBUILD_SHARED_LIBS="$BUILD_SHARED_LIBS"
    -DBUILD_EXAMPLES="$BUILD_EXAMPLES"
    -DBUILD_TESTS="$BUILD_TESTS"
    -DENABLE_NETWORK="$ENABLE_NETWORK"
)

cmake .. "${CMAKE_ARGS[@]}"

# Build
echo ""
print_section "Building Project"
echo -e "${BLUE}Compiling...${NC}"
CPU_COUNT=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
cmake --build . --config "$BUILD_TYPE" -j$CPU_COUNT

# Check build success
if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║        Build Successful! ✓            ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
    echo ""
    
    print_section "Build Output"
    if [ "$OS" == "Darwin" ]; then
        if [ -f "lib/libdmusicpak.dylib" ]; then
            echo -e "  ${GREEN}✓${NC} Library: $BUILD_DIR/lib/libdmusicpak.dylib"
        fi
    else
        if [ -f "lib/libdmusicpak.so" ]; then
            echo -e "  ${GREEN}✓${NC} Library: $BUILD_DIR/lib/libdmusicpak.so"
        fi
    fi
    
    if [ "$BUILD_EXAMPLES" = "ON" ]; then
        echo ""
        echo -e "${CYAN}Examples:${NC}"
        for example in bin/*; do
            if [ -f "$example" ] && [ -x "$example" ]; then
                echo -e "  ${GREEN}✓${NC} $(basename $example)"
            fi
        done
    fi
    
    echo ""
    print_section "Next Steps"
    if [ "$BUILD_EXAMPLES" = "ON" ]; then
        echo "  Run examples:"
        echo "    cd $BUILD_DIR/bin"
        echo "    ./write_example"
        echo "    ./read_example example.dmusicpak"
        if [ "$ENABLE_NETWORK" = "ON" ]; then
            echo "    ./example_network"
        fi
        echo ""
    fi
    
    echo "  Install library:"
    echo "    sudo cmake --install ."
    echo ""
    
    # Offer to run examples
    if [ "$BUILD_EXAMPLES" = "ON" ]; then
        if ask_yn "Run write example now?" "n"; then
            cd bin
            if [ -f "./write_example" ]; then
                ./write_example
                echo ""
                echo -e "${GREEN}✓${NC} Example package created: example.dmusicpak"
                
                if ask_yn "Run read example?" "n"; then
                    if [ -f "./read_example" ]; then
                        ./read_example example.dmusicpak
                    fi
                fi
            fi
        fi
    fi
    
    echo ""
    echo -e "${GREEN}Build completed successfully!${NC}"
else
    echo ""
    echo -e "${RED}╔════════════════════════════════════════╗${NC}"
    echo -e "${RED}║          Build Failed! ✗              ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════╝${NC}"
    exit 1
fi
