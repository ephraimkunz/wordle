#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

const int WORD_SIZE = 5;
const int WORD_SIZE_WITH_TERMINATOR = WORD_SIZE + 1;
const int ALPHABET_LENGTH = 26;

typedef struct {
  size_t count;
  char *words;
} wordlist;

// TODO: Handle placement_args.

int init_wordlist(wordlist *list) {
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

  if (madvise(list->words, list->count, MADV_SEQUENTIAL) == -1) {
    perror("Unable to madvise sequential");
    return -1;
  }

  return 0;
}

int deinit_wordlist(wordlist *list) {
  if (munmap(list->words, list->count) == -1) {
    perror("Unable to unmap memory");
    return -1;
  }

  return 0;
}

void usage(void) {
  printf("wordle <required_arg> <forbidden_arg> <placement_arg>\n\
    required_arg: 5 character string specifying locations of required characters, - for blanks. Ex: \"br-a-\"\n\
    forbidden_arg: list of characters forbidden in the final word. Ex: \"qyz\"\n\
    placement_arg: <character><allowed disallowed location><disallowed location>, 0-indexed. Ex: \"e03d123\"\n");
}

int main(int argc, char **argv) {
  if (argc != 4) {
    usage();
    return -1;
  }

  // Setup required.
  char required[WORD_SIZE];
  assert(strlen(argv[1]) == WORD_SIZE);
  for (int i = 0; i < WORD_SIZE; ++i) {
    char letter = argv[1][i];
    assert(letter == '-' || (letter >= 'a' && letter <= 'z'));
    required[i] = letter == '-' ? 0 : letter;
  }

  // Setup forbidden.
  bool forbidden[ALPHABET_LENGTH];
  for (int i = 0; i < ALPHABET_LENGTH; ++i) {
    forbidden[i] = false;
  }
  char *forbidden_char = argv[2];
  while (*forbidden_char != 0) {
    assert(*forbidden_char >= 'a' && *forbidden_char <= 'z');
    forbidden[*forbidden_char - 'a'] = true;
    forbidden_char++;
  }

  // Setup wordlist.

  wordlist list;
  if (init_wordlist(&list) == -1) {
    return -1;
  }

  for (size_t w = 0; w < list.count; w++) {
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
    int placement_length = strlen(argv[3]);
    char placement = 0;
    for (int i = 0; i < placement_length; ++i) {
      char c = argv[3][i];
      if (isalpha(c)) {
        assert(c >= 'a' && c <= 'z');

        // When switching required letters, check that the last required letter,
        // if present, is found somewhere in the word.
        if (i > 0) {
          assert(placement >= 'a' && placement <= 'z');

          bool found_somewhere = false;
          for (int j = 0; j < WORD_SIZE; ++j) {
            if (required[j] == 0 && placement == list.words[w * WORD_SIZE_WITH_TERMINATOR + j]) {
              found_somewhere = true;
              break;
            }
          }

          if (!found_somewhere) {
            valid = false;
            break;
          }
        }

        placement = c;
      } else {
        // Check that letter not in disallowed position.
        assert(placement >= 'a' && placement <= 'z');
        int disallowed_index = c - '0';
        assert(disallowed_index < 5 && disallowed_index >= 0);
        if (placement ==
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
    if (placement > 0) {
      bool has_placement = false;
      for (int j = 0; j < WORD_SIZE; ++j) {
        if (required[j] == 0 && placement == list.words[w * WORD_SIZE_WITH_TERMINATOR + j]) {
          has_placement = true;
          break;
        }
      }

      valid = has_placement;
    }

    if (!valid) {
      continue;
    }

    // Print the candidate.
    printf("%.*s\n", WORD_SIZE, &list.words[w * WORD_SIZE_WITH_TERMINATOR]);
  }

  return deinit_wordlist(&list);
}
