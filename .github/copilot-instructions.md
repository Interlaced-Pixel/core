# Interlaced Core Copilot Instructions

## Architecture Overview
Interlaced Core is a header-only C++ library providing modular utilities for filesystem operations, JSON handling, logging, and networking. The library is structured as an interface library in CMake, with all headers in the `include/` directory. Key components include:
- `filesystem.hpp`: File system utilities
- `json.hpp`: JSON parsing and manipulation
- `logging.hpp`: Logging mechanisms
- `network.hpp`: Network-related functions

The "why" behind the header-only design is to simplify integration without linking, allowing users to include only necessary headers.

## Build System
- Use Makefile with C++17 standard.
- Root `Makefile` sets up the test build and coverage.
- Build commands: `make test` to build, `make run-tests` to run, `make coverage` for coverage.
- Compile commands are exported for IDE integration.

## Testing
Unit tests are included and should achieve at least 95% code coverage.

## Developer Workflows
- CI/CD via GitHub Actions in `.github/workflows/build-test.yml` (assumes standard build and test steps).
- Debug builds: Add `-O0 -g` to CXXFLAGS in Makefile.
- Dependencies: Doctest is fetched automatically; no other third-party libraries required.

## Conventions and Patterns
- Header files use `.hpp` extension, placed directly in `include/`.
- Test files use `.cc` extension in `tests/`.
- Makefile structure: Root Makefile for building tests and coverage.
- Doctest is used for unit testing.

## Integration Points
- Tests are implemented using doctest under `tests/` and linked via Makefile.
- No cross-component communication beyond header includes; each module is self-contained.
- Doctest is fetched automatically via `tools/get_doctest.sh`.

## Rules & Restrictions
- No third-party dependencies are required; the testing helpers are part of this repository.
- Maintain header-only design; no compiled binaries.
- Follow C++17 standard; no newer features.
- Aim for at least 95% code coverage for unit tests.