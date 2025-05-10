# Compiler and flags
CC := clang
CFLAGS := -Wall -Wextra -Wpedantic -std=c17 -O3
SANITIZE_FLAGS := -fsanitize=address,undefined -g
COVERAGE_FLAGS := -O0 -g -fprofile-instr-generate -fcoverage-mapping

# Sources
SRC := wordle.c
HDR := wordle.h
MAIN := main.c
TEST := test.c

# Binaries
MAIN_BIN := wordle
TEST_BIN := test_wordle
MAIN_SAN := wordle_san
TEST_SAN := test_wordle_san

# Coverage
PROFILE_RAW := $(TEST_BIN).profraw
PROFILE_DATA := $(TEST_BIN).profdata
COVERAGE_DIR := coverage-html

.PHONY: all main test sanitize coverage-html clean

# Default build
all: main

# Regular optimized build
main: $(SRC) $(HDR) $(MAIN)
	$(CC) $(CFLAGS) $(SRC) $(MAIN) -o $(MAIN_BIN)

test: $(SRC) $(HDR) $(TEST)
	$(CC) $(CFLAGS) $(SRC) $(TEST) -o $(TEST_BIN)
	./$(TEST_BIN)

# Sanitizer builds
sanitize: $(MAIN_SAN) $(TEST_SAN)

$(MAIN_SAN): $(SRC) $(HDR) $(MAIN)
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) $(SRC) $(MAIN) -o $@

$(TEST_SAN): $(SRC) $(HDR) $(TEST)
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) $(SRC) $(TEST) -o $@

# Code coverage build (HTML only)
coverage: clean
	$(CC) $(COVERAGE_FLAGS) $(SRC) $(TEST) -o $(TEST_BIN)
	LLVM_PROFILE_FILE="$(PROFILE_RAW)" ./$(TEST_BIN)
	xcrun llvm-profdata merge -sparse $(PROFILE_RAW) -o $(PROFILE_DATA)
	xcrun llvm-cov show ./$(TEST_BIN) \
		-instr-profile=$(PROFILE_DATA) \
		-format=html -output-dir=$(COVERAGE_DIR) $(SRC)
	open $(COVERAGE_DIR)/index.html

# Clean everything
clean:
	rm -f $(MAIN_BIN) $(TEST_BIN) $(MAIN_SAN) $(TEST_SAN)
	rm -f *.profraw *.profdata
	rm -rf $(COVERAGE_DIR)
	rm -rf *.dSYM
