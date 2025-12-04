#!/bin/bash
# Pina Engine - macOS Build Script
# Usage: ./build-macos.sh [--debug|--release]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Default values
BUILD_TYPE=""
CLEAN_BUILD=false
JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        -j*)
            JOBS="${1#-j}"
            shift
            ;;
        --help|-h)
            echo "Pina Engine - macOS Build Script"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --debug     Build in Debug mode"
            echo "  --release   Build in Release mode"
            echo "  --clean     Clean build directory before building"
            echo "  -j<N>       Number of parallel jobs (default: CPU cores)"
            echo "  -h, --help  Show this help message"
            echo ""
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Prompt for build type if not specified
if [ -z "$BUILD_TYPE" ]; then
    echo -e "${BLUE}Pina Engine - macOS Build${NC}"
    echo ""
    echo "Select build type:"
    echo "  1) Debug"
    echo "  2) Release"
    echo ""
    read -p "Enter choice [1-2]: " choice
    case $choice in
        1)
            BUILD_TYPE="Debug"
            ;;
        2)
            BUILD_TYPE="Release"
            ;;
        *)
            echo -e "${RED}Invalid choice. Defaulting to Debug.${NC}"
            BUILD_TYPE="Debug"
            ;;
    esac
fi

BUILD_DIR="build-macos-$(echo $BUILD_TYPE | tr '[:upper:]' '[:lower:]')"

echo ""
echo -e "${BLUE}==================================${NC}"
echo -e "${BLUE}  Pina Engine - macOS Build${NC}"
echo -e "${BLUE}==================================${NC}"
echo -e "  Build Type: ${GREEN}$BUILD_TYPE${NC}"
echo -e "  Build Dir:  ${GREEN}$BUILD_DIR${NC}"
echo -e "  Jobs:       ${GREEN}$JOBS${NC}"
echo -e "${BLUE}==================================${NC}"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake not found.${NC}"
    echo "Please install CMake: brew install cmake"
    exit 1
fi

# Clean build if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure
echo -e "${YELLOW}Configuring...${NC}"
cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -G "Unix Makefiles"

# Build
echo -e "${YELLOW}Building...${NC}"
cmake --build "$BUILD_DIR" -j "$JOBS"

# Copy compile_commands.json to root for IDE support
if [ -f "$BUILD_DIR/compile_commands.json" ]; then
    cp "$BUILD_DIR/compile_commands.json" .
fi

echo ""
echo -e "${GREEN}==================================${NC}"
echo -e "${GREEN}  Build complete!${NC}"
echo -e "${GREEN}==================================${NC}"
echo ""
echo "Binaries are in: $BUILD_DIR/bin/"
echo ""

# List built executables
if [ -d "$BUILD_DIR/bin" ]; then
    echo "Built executables:"
    ls -la "$BUILD_DIR/bin/" 2>/dev/null | grep -E "^-" | awk '{print "  - " $NF}'
fi
