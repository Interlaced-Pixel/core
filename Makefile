CXX ?= clang++
STD ?= c++17
WARN := -Wall -Wextra -Wpedantic
DBG := -O0 -g
INCLUDES := -Iinclude -Ithird_party/doctest

# LLVM/Clang coverage on macOS
COV_CFLAGS := -fprofile-instr-generate -fcoverage-mapping
COV_ENV := LLVM_PROFILE_FILE

# Use Xcode toolchain wrappers on macOS to locate llvm tools
LLVM_PROFDATA := xcrun llvm-profdata
LLVM_COV := xcrun llvm-cov

TEST_DIR := tests
BIN_DIR := build/tests
COVERAGE_DIR := coverage

SOURCES := $(wildcard $(TEST_DIR)/*.cc)
TEST_BIN := $(BIN_DIR)/interlaced_core_tests

.PHONY: all test run-tests coverage clean doctest

all: test

$(TEST_BIN): $(SOURCES) third_party/doctest/doctest.h | $(BIN_DIR)
	$(CXX) -std=$(STD) $(WARN) $(DBG) $(INCLUDES) $(COV_CFLAGS) $(SOURCES) -o $(TEST_BIN)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

third_party/doctest/doctest.h:
	@mkdir -p third_party/doctest
	@./tools/get_doctest.sh

test: $(TEST_BIN)
	@echo "Built test binary: $(TEST_BIN)"

run-tests: test
	@set -e; \
	$(COV_ENV)="$(TEST_BIN).profraw" "$(TEST_BIN)"

coverage: run-tests
	@mkdir -p $(COVERAGE_DIR)/html
	@echo "Merging raw profiles -> $(COVERAGE_DIR)/coverage.profdata";
	@$(LLVM_PROFDATA) merge -sparse $(BIN_DIR)/*.profraw -o $(COVERAGE_DIR)/coverage.profdata
	@echo "Exporting lcov -> $(COVERAGE_DIR)/lcov.info";
	@$(LLVM_COV) export --format=lcov --ignore-filename-regex="third_party/.*" $(TEST_BIN) -instr-profile=$(COVERAGE_DIR)/coverage.profdata > $(COVERAGE_DIR)/lcov.info
	@echo "Generating HTML -> $(COVERAGE_DIR)/html";
	@$(LLVM_COV) show --ignore-filename-regex="third_party/.*" $(TEST_BIN) -instr-profile=$(COVERAGE_DIR)/coverage.profdata -format=html -output-dir=$(COVERAGE_DIR)/html -show-expansions -show-line-counts-or-regions
	@echo "Open: $(COVERAGE_DIR)/html/index.html"

clean:
	rm -rf $(BIN_DIR) $(COVERAGE_DIR)
