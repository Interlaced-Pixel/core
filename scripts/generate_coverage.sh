#!/bin/bash
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

cd "$BUILD_DIR"

echo "Merging coverage data..."
xcrun llvm-profdata merge -sparse tests/*.profraw -o coverage.profdata

echo "Generating coverage for logging..."
xcrun llvm-cov export -format=lcov -instr-profile=coverage.profdata tests/logging_test -ignore-filename-regex='.*_deps/.*' > coverage_logging.info

echo "Generating coverage for json..."
xcrun llvm-cov export -format=lcov -instr-profile=coverage.profdata tests/json_test -ignore-filename-regex='.*_deps/.*' > coverage_json.info

echo "Merging coverage reports..."
cat coverage_logging.info coverage_json.info > coverage.info

echo "Coverage report generated: $BUILD_DIR/coverage.info"
