# pixelLib — AI Agent Instructions

## Purpose
Comprehensive contributor guidance for this header-only C++23 utility library providing filesystem, JSON, logging, and network operations with modern development practices.

## Quick Start (build & run)
- Build everything: `make` (builds test binary with coverage enabled)
- Debug builds: Add `-O0 -g` to `CXXFLAGS` in the Makefile or run `make CXXFLAGS="-O0 -g"`
- Run unit tests: `make run-tests` (executes all test suites)
- Coverage: `make coverage` → produces HTML report in `build/coverage/` and LCOV files
- Code quality: `make clang-tidy` (static analysis), `make clang-tidy-fix` (auto-fixes)

## Big Picture (Architecture)
- **Header-only design**: All public headers in `include/` with no linking required
- **C++23 target**: Modern C++ features with clang-tidy enforcement for best practices
- **Core modules**: `filesystem.hpp`, `json.hpp`, `logging.hpp`, `network.hpp`
- **Namespace**: `pixellib::core::<module>` (e.g., `pixellib::core::json::JSON`)
- **Testing**: Single test binary using doctest, comprehensive coverage with gcov/lcov

## Project Conventions & Patterns
- **C++ Standard**: Target C++23 (not C++17), enforced by Makefile (`STD ?= c++23`)
- **Headers**: `.hpp` extension in `include/` directory, no subdirectories
- **Tests**: `.cc` extension in `tests/` directory, one test file per module
- **Minimal dependencies**: Only doctest for testing (fetched via `tools/get_doctest.sh`)
- **Namespace structure**: `pixellib::core::<module>::<class>` pattern
- **Self-contained modules**: Each header is independently usable
- **Modern C++ practices**: clang-tidy enforces modernize-* and performance-* checks

## Build System & Testing
- **Makefile-driven**: Single Makefile handles all build, test, and coverage tasks
- **Compiler detection**: Auto-detects clang/gcc/Windows and adjusts flags accordingly
- **Coverage support**: LLVM/clang coverage on macOS, GCC/gcovr on Linux
- **Test isolation**: Set `PIXELLIB_TEST_MODE=1` for deterministic offline-safe tests
- **Compilation database**: `make compile-commands` generates `build/compile_commands.json`

## Code Quality & Tooling
- **clang-tidy**: Comprehensive configuration in `.clang-tidy` with modern C++ enforcement
- **clang-format**: Code formatting via `.clang-format` file
- **VS Code integration**: Optimized settings in `.vscode/` for clangd and clang-tidy
- **Static analysis**: `make clang-tidy-report` generates detailed analysis reports
- **Coverage reports**: HTML output in `build/coverage/html/` with workspace-relative LCOV

## Development Workflow
- **Local development**: Use `make run-tests` for quick validation
- **Debug builds**: Modify `CXXFLAGS` in Makefile or override on command line
- **Coverage analysis**: `make coverage` then open `build/coverage/html/index.html`
- **Code quality**: Run `make clang-tidy` before committing changes
- **CI/CD**: GitHub Actions in `.github/workflows/build-test.yml` runs all checks

## File Structure & Organization
```
pixelLib/
├── include/           # Public headers (header-only)
│   ├── filesystem.hpp
│   ├── json.hpp
│   ├── logging.hpp
│   └── network.hpp
├── tests/            # Unit tests (one per module)
│   ├── test_filesystem.cc
│   ├── test_json.cc
│   ├── test_logging.cc
│   ├── test_network.cc
│   └── test_main.cc
├── tools/            # Utility scripts
│   ├── get_doctest.sh
│   ├── run-clang-tidy.sh
│   └── setup-clang-tidy.sh
├── third-party/      # Vendored dependencies
│   └── doctest/
└── build/           # Build artifacts (generated)
    ├── tests/
    ├── coverage/
    └── compile_commands.json
```

## Key Implementation Details
- **Filesystem**: Cross-platform operations with Windows/Unix path handling
- **JSON**: DOM parser with Unicode support, strict validation, configurable serialization
- **Logging**: Multi-level with file rotation, thread-safe, structured logging support
- **Network**: HTTP/HTTPS operations, hostname resolution, download with progress tracking

## Testing Guidelines
- **High coverage target**: Aim for 95%+ code coverage
- **Deterministic tests**: Use `PIXELLIB_TEST_MODE=1` for offline-safe network operations
- **Single file per module**: Keep all tests for a module in one `tests/test_<module>.cc` file
- **Doctest macros**: Use `TEST_CASE`, `CHECK`, `REQUIRE` from doctest
- **Test isolation**: Tests should be self-contained with no external service dependencies

## Rules for Code Changes
- **Header-only preservation**: No compiled library artifacts, maintain header-only design
- **C++23 compliance**: Use modern C++ features enforced by clang-tidy
- **Minimal dependencies**: Only add dependencies if absolutely necessary
- **Consistent style**: Follow existing naming conventions and clang-format rules
- **Test coverage**: Add comprehensive tests for any new functionality
- **Performance**: Consider performance implications; clang-tidy will flag inefficiencies
- **Cross-platform**: Ensure Windows/macOS/Linux compatibility

## Integration Points
- **VS Code**: Optimized configuration with clangd, clang-tidy, and clang-format-2025
- **clang-tidy**: Extensive check suite for modern C++, performance, and safety
- **Coverage**: LLVM coverage on macOS, GCC/gcovr on Linux with HTML output
- **CI**: GitHub Actions runs build, test, coverage, and clang-tidy checks

## Notes for AI Agents
- **Focus on root causes**: Use clang-tidy to identify and fix fundamental issues
- **Modern C++ practices**: Leverage C++23 features and follow clang-tidy recommendations
- **Header-only constraints**: All implementations must be in headers, no source files
- **Test-driven**: Always add/modify tests when implementing changes
- **Performance awareness**: clang-tidy performance checks help maintain efficiency
- **Cross-platform consideration**: Test on multiple platforms when making changes
