#!/bin/bash
# clang-tidy runner script for pixelLib project
# Usage: ./tools/run-clang-tidy.sh [options]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project root
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# Default options
CLANG_TIDY="clang-tidy"
BUILD_DIR="build"
FIX=false
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo "4")
VERBOSE=false
OUTPUT_FORMAT="text"
OUTPUT_FILE=""
CHECKS="*"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -f|--fix)
            FIX=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -o|--output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        --format)
            OUTPUT_FORMAT="$2"
            shift 2
            ;;
        --checks)
            CHECKS="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  -j, --jobs N         Number of parallel jobs (default: $JOBS)"
            echo "  -f, --fix            Apply fixes automatically"
            echo "  -v, --verbose        Verbose output"
            echo "  -o, --output FILE    Output to file"
            echo "  --format FORMAT      Output format: text, yaml, or markdown (default: text)"
            echo "  --checks CHECKS      Comma-separated list of checks (default: *)"
            echo "  --build-dir DIR      Build directory (default: $BUILD_DIR)"
            echo "  -h, --help           Show this help"
            echo ""
            echo "Examples:"
            echo "  $0                           # Run all checks"
            echo "  $0 -f                        # Run checks and apply fixes"
            echo "  $0 -j 8 --checks bugprone-*  # Run only bugprone checks with 8 jobs"
            echo "  $0 --format markdown -o report.md  # Generate markdown report"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

# Check if clang-tidy is available
if ! command -v "$CLANG_TIDY" &> /dev/null; then
    echo -e "${RED}Error: clang-tidy not found. Please install clang-tidy.${NC}"
    echo "On macOS: brew install llvm"
    echo "On Ubuntu: sudo apt install clang-tidy"
    exit 1
fi

# Check if .clang-tidy exists
if [[ ! -f "$PROJECT_ROOT/.clang-tidy" ]]; then
    echo -e "${RED}Error: .clang-tidy configuration not found in project root.${NC}"
    exit 1
fi

# Ensure build directory exists
if [[ ! -d "$BUILD_DIR" ]]; then
    echo -e "${YELLOW}Build directory '$BUILD_DIR' not found. Creating it...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Find source files
echo -e "${BLUE}Finding source files...${NC}"
SOURCE_FILES=$(find . -name "*.cpp" -o -name "*.cc" -o -name "*.c" | \
    grep -v "third-party/" | \
    grep -v "build/" | \
    grep -v ".git/" | \
    sort)

if [[ -z "$SOURCE_FILES" ]]; then
    echo -e "${YELLOW}No source files found to analyze.${NC}"
    exit 0
fi

# Count files
FILE_COUNT=$(echo "$SOURCE_FILES" | wc -l)
echo -e "${BLUE}Found $FILE_COUNT source files to analyze${NC}"

# Prepare clang-tidy command
CLANG_TIDY_CMD="$CLANG_TIDY"
CLANG_TIDY_CMD="$CLANG_TIDY_CMD --checks='$CHECKS'"
CLANG_TIDY_CMD="$CLANG_TIDY_CMD -j $JOBS"

if [[ "$FIX" == "true" ]]; then
    CLANG_TIDY_CMD="$CLANG_TIDY_CMD --fix"
    echo -e "${YELLOW}Running with automatic fixes enabled${NC}"
fi

if [[ "$VERBOSE" == "true" ]]; then
    CLANG_TIDY_CMD="$CLANG_TIDY_CMD --verbose"
fi

# Prepare header filter and compile commands
HEADER_FILTER="--header-filter='^(include/|tests/)'"
COMPILE_COMMANDS_OPTION="--export-fixes=clang-tidy-fixes.yaml"

# Convert SOURCE_FILES to array (compatible with bash < 4.0)
IFS=$'\n' read -r -d '' -a FILES <<< "$SOURCE_FILES"
unset IFS

echo -e "${BLUE}Running clang-tidy...${NC}"
echo "Command: ${CLANG_TIDY} --checks='${CHECKS}' ${HEADER_FILTER} ${FIX_OPTION} ${COMPILE_COMMANDS_OPTION} ${FILES[@]}"

if [[ -n "$OUTPUT_FILE" && "$OUTPUT_FORMAT" == "text" ]]; then
    ${CLANG_TIDY} --checks="${CHECKS}" ${HEADER_FILTER} ${FIX_OPTION} ${COMPILE_COMMANDS_OPTION} "${FILES[@]}" 2>&1 | tee "$OUTPUT_FILE"
else
    ${CLANG_TIDY} --checks="${CHECKS}" ${HEADER_FILTER} ${FIX_OPTION} ${COMPILE_COMMANDS_OPTION} "${FILES[@]}" 2>&1
fi

# Check results
EXIT_CODE=$?

if [[ $EXIT_CODE -eq 0 ]]; then
    echo -e "${GREEN}✓ clang-tidy completed successfully${NC}"
    if [[ "$FIX" == "true" ]]; then
        echo -e "${GREEN}✓ Fixes have been applied${NC}"
    fi
else
    echo -e "${RED}✗ clang-tidy found issues${NC}"
    if [[ "$FIX" != "true" ]]; then
        echo -e "${YELLOW}Run with -f to apply automatic fixes${NC}"
    fi
fi

# Generate summary report if requested
if [[ -n "$OUTPUT_FILE" && "$OUTPUT_FORMAT" == "markdown" ]]; then
    echo -e "${BLUE}Generating markdown report...${NC}"
    cat > "$OUTPUT_FILE" << EOF
# clang-tidy Report

**Generated:** $(date)
**Project:** pixelLib
**Files analyzed:** $FILE_COUNT
**Checks run:** $CHECKS

## Summary

EOF

    if [[ $EXIT_CODE -eq 0 ]]; then
        echo "✅ All checks passed!" >> "$OUTPUT_FILE"
    else
        echo "❌ Issues found. See detailed output above." >> "$OUTPUT_FILE"
    fi

    echo -e "${GREEN}Report generated: $OUTPUT_FILE${NC}"
fi

exit $EXIT_CODE
