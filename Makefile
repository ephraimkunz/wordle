# Makefile for Wordle CLI and testing with sanitizers and header support

CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=c17 -O2
SANITIZE_FLAGS := -fsanitize=address,undefined -g

SRC := wordle.c
HDR := wordle.h
MAIN := main.c
TEST := test.c

MAIN_BIN := wordle
TEST_BIN := test_wordle
MAIN_BIN_SAN := wordle_san
TEST_BIN_SAN := test_wordle_san

.PHONY: all main test sanitize clean

main: $(MAIN_BIN)

$(MAIN_BIN): $(SRC) $(HDR) $(MAIN)
	$(CC) $(CFLAGS) $(SRC) $(MAIN) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(SRC) $(HDR) $(TEST)
	$(CC) $(CFLAGS) $(SRC) $(TEST) -o $@

sanitize: $(MAIN_BIN_SAN) $(TEST_BIN_SAN)

$(MAIN_BIN_SAN): $(SRC) $(HDR) $(MAIN)
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) $(SRC) $(MAIN) -o $@

$(TEST_BIN_SAN): $(SRC) $(HDR) $(TEST)
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) $(SRC) $(TEST) -o $@

clean:
	rm -rf $(MAIN_BIN) $(TEST_BIN) $(MAIN_BIN_SAN) $(TEST_BIN_SAN) test_wordle_san.dSYM wordle_san.dSYM
