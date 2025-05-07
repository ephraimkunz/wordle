#include <stdlib.h>

// Populate buffer with candidate words for wordle, given constraints.
// buffer: buffer in which to insert words.
// size: size of the buffer in bytes
// required_arg: string of the form "b-ea-", which indicates locations of correct letters.
// fobidden_arg: string of the form "askecsl", which indicates forbidden letters.
// placement_arg: string of the form "<letter><disallowed positions><letter><disallowed positions>..."
// Used to track letters that are required in the word, but not in certain positions.
// return value: number of bytes used in buffer. 0 indicates no matches, or an error. Use wordle_get_nth_word
// to extract words from the buffer.
size_t wordle_wordlist(char *buffer, size_t size, char *required_arg, char *forbidden_arg, char *placement_arg);

// Extract the nth word stored in the buffer.
// buffer: buffer populated by wordle_wordlist
// size: size in bytes of the buffer
// n: 0-based index of the word to extract
// return value: pointer to the null-terminated word, or NULL if invalid.
char * wordle_get_nth_word(char *buffer, size_t size, size_t n);
