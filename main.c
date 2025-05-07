#include "wordle.h"
#include <stdio.h>

static void usage(void) {
    printf("wordle <required_arg> <forbidden_arg> <placement_arg>\n\
      required_arg: 5 character string specifying locations of required characters, - for blanks. Ex: \"br-a-\"\n\
      forbidden_arg: list of characters forbidden in the final word. Ex: \"qyz\"\n\
      placement_arg: <character><disallowed location><disallowed location>, 0-indexed. Ex: \"e03d123\"\n");
  }  

int main(int argc, char **argv) {
    if (argc != 4) {
      usage();
      return -1;
    }
  
    size_t buf_size = 100;
    char buf[buf_size];
  
    size_t count = wordle_wordlist(buf, buf_size, argv[1], argv[2], argv[3]);
    if (count > 0) {
      char *word;
      size_t i = 0;
      while ((word = wordle_get_nth_word(buf, count, i)) != NULL) {
        printf("%s\n", word);
        i += 1;
      }
    }
  }
