#include "wordle.h"
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static const int WORD_SIZE = 5;
static const int WORD_SIZE_WITH_TERMINATOR = WORD_SIZE + 1;
static const int ALPHABET_LENGTH = 26;

typedef struct {
  size_t count;
  char *words;
} wordlist;

static int init_wordlist(wordlist *list) {
  // Wordlist from
  // https://gist.github.com/dracos/dd0668f281e685bad51479e5acaadb93
  int file = open("words.txt", O_RDONLY);
  if (file == -1) {
    perror("Unable to open word list file");
    return -1;
  }

  struct stat info;
  if (fstat(file, &info) == -1) {
    perror("Unable to get word list file length");
    close(file);
    return -1;
  }
  list->count = info.st_size / WORD_SIZE_WITH_TERMINATOR;

  if ((list->words = mmap(NULL, info.st_size, PROT_READ, MAP_PRIVATE, file,
                          0)) == MAP_FAILED) {
    perror("Unable to mmap word list file");
    close(file);
    return -1;
  }

  close(file);

  if (madvise(list->words, info.st_size, MADV_SEQUENTIAL) == -1) {
    perror("Unable to madvise sequential");
    return -1;
  }

  return 0;
}

static int deinit_wordlist(wordlist *list) {
  if (munmap(list->words, list->count * WORD_SIZE_WITH_TERMINATOR) == -1) {
    perror("Unable to unmap memory");
    return -1;
  }

  return 0;
}

char *wordle_get_nth_word(char *buffer, size_t size, size_t n) {
  if (buffer == NULL || size == 0) {
    return NULL;
  }

  // End of word not past end of buffer.
  if (WORD_SIZE_WITH_TERMINATOR * n + WORD_SIZE_WITH_TERMINATOR > size) {
    return NULL;
  }

  return &buffer[WORD_SIZE_WITH_TERMINATOR * n];
}

// placement_arg format: letter followed by 1+ indices where it cannot appear.
// e.g., "a12b0" means 'a' cannot be in pos 1 or 2, 'b' cannot be in pos 0.
// (but each of a and b must be somewhere else in the word)
size_t wordle_wordlist(char *buffer, size_t size, char *required_arg,
                       char *forbidden_arg, char *placement_arg) {
  // Check args
  if (buffer == NULL || size == 0 || required_arg == NULL ||
      forbidden_arg == NULL || placement_arg == NULL) {
    return 0;
  }

  // Setup required.
  char required[WORD_SIZE];
  memset(required, 0, sizeof(required));
  if (strlen(required_arg) != WORD_SIZE) {
    return 0;
  }
  for (int i = 0; i < WORD_SIZE; ++i) {
    char letter = required_arg[i];
    if (!(letter == '-' || islower(letter))) {
      return 0;
    }
    required[i] = letter == '-' ? 0 : letter;
  }

  // Setup forbidden.
  bool forbidden[ALPHABET_LENGTH];
  memset(forbidden, 0, sizeof(forbidden));

  char *forbidden_char = forbidden_arg;
  while (*forbidden_char != 0) {
    if (!islower(*forbidden_char)) {
      return 0;
    }
    forbidden[*forbidden_char - 'a'] = true;
    forbidden_char++;
  }

  // Setup wordlist.
  wordlist list;
  if (init_wordlist(&list) == -1) {
    return 0;
  }

  size_t used_size = 0;
  for (size_t w = 0; (w < list.count) && (size > used_size) &&
                     ((size - used_size) >= WORD_SIZE_WITH_TERMINATOR);
       w++) {
    bool valid = true;

    // Check required and forbidden.
    for (int i = 0; i < WORD_SIZE; ++i) {
      char c = list.words[w * WORD_SIZE_WITH_TERMINATOR + i];
      if (required[i] == 0) {
        if (forbidden[c - 'a']) {
          valid = false;
          break;
        }
      } else if (required[i] != c) {
        valid = false;
        break;
      }
    }

    if (!valid) {
      continue;
    }

    // Check placement arg.
    int placement_length = strlen(placement_arg);
    char current_letter = 0;
    for (int i = 0; i < placement_length; ++i) {
      char c = placement_arg[i];
      if (isalpha(c)) {
        if (!islower(c)) {
          return 0;
        }

        // When switching required letters, check that the last required letter,
        // if present, is found somewhere in the word.
        if (i > 0) {
          bool found_somewhere = false;
          for (int j = 0; j < WORD_SIZE; ++j) {
            if (required[j] == 0 &&
                current_letter ==
                    list.words[w * WORD_SIZE_WITH_TERMINATOR + j]) {
              found_somewhere = true;
              break;
            }
          }

          if (!found_somewhere) {
            valid = false;
            break;
          }
        }

        current_letter = c;
      } else {
        // Check that letter not in disallowed position.
        if (!islower(current_letter)) {
          return 0;
        }
        int disallowed_index = c - '0';
        if (disallowed_index >= WORD_SIZE || disallowed_index < 0) {
          return 0;
        }
        if (current_letter ==
            list.words[w * WORD_SIZE_WITH_TERMINATOR + disallowed_index]) {
          valid = false;
          break;
        }
      }
    }

    if (!valid) {
      continue;
    }

    // Check that the last required letter, if present, is found somewhere
    // in the word.
    if (isalpha(current_letter)) { // We've actually set current_letter, meaning we've entered the above loop.
      bool has_placement = false;
      for (int j = 0; j < WORD_SIZE; ++j) {
        if (required[j] == 0 &&
            current_letter == list.words[w * WORD_SIZE_WITH_TERMINATOR + j]) {
          has_placement = true;
          break;
        }
      }

      valid = has_placement;
    }

    if (!valid) {
      continue;
    }

    // Output the candidate.
    for (int i = 0; i < WORD_SIZE; ++i) {
      buffer[used_size] = list.words[w * WORD_SIZE_WITH_TERMINATOR + i];
      used_size++;

      // Insert null terminator if we're at the end.
      if (i == (WORD_SIZE - 1)) {
        buffer[used_size] = 0;
        used_size++;
      }
    }
  }

  deinit_wordlist(&list);

  return used_size;
}
