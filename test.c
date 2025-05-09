#include "wordle.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_nth_word_invalid_args(void) {
  assert(wordle_get_nth_word(NULL, 1, 1) == NULL && "Null buf");

  char buf[5];
  assert(wordle_get_nth_word(buf, 0, 0) == NULL && "Zero length buf");
  assert(wordle_get_nth_word(buf, 5, 0) == NULL && "Too small buf");

  char buf2[6];
  assert(wordle_get_nth_word(buf2, 0, 0) == NULL && "Zero length buf");
  assert(wordle_get_nth_word(buf2, 6, 0) != NULL && "Right size buf");

  char buf3[100];
  assert(wordle_get_nth_word(buf3, 100, 101) == NULL && "Too big n");
}

void test_wordlist_invalid_args(void) {
  assert(wordle_wordlist(NULL, 10, "bread", "", "") == 0 && "Null buf");

  char buf[6];
  assert(wordle_wordlist(buf, 0, "bread", "", "") == 0 && "Zero length buf");

  assert(wordle_wordlist(buf, 6, "bred", "", "") == 0 && "Too short required");
  assert(wordle_wordlist(buf, 6, "breads", "", "") == 0 && "Too long required");
  assert(wordle_wordlist(buf, 6, "bre_d", "", "") == 0 &&
         "Invalid letter required");
  assert(wordle_wordlist(buf, 6, NULL, "", "") == 0 && "Null required");

  assert(wordle_wordlist(buf, 6, "bread", "_", "") == 0 &&
         "Invalid letter forbidden");
  assert(wordle_wordlist(buf, 6, "bread", "1", "") == 0 &&
         "Invalid letter forbidden 2");
  assert(wordle_wordlist(buf, 6, "bread", NULL, "") == 0 && "Null forbidden");

  assert(wordle_wordlist(buf, 6, "bread", "", "_") == 0 &&
         "Invalid letter placement");
  assert(wordle_wordlist(buf, 6, "bread", "", ":") == 0 &&
         "Invalid letter placement 2");
  assert(wordle_wordlist(buf, 6, "bread", "", NULL) == 0 && "Null placement");
  assert(wordle_wordlist(buf, 6, "br-ad", "e9", NULL) == 0 && "Invalid digit");
  assert(wordle_wordlist(buf, 6, "br-ad", "e6", NULL) == 0 &&
         "Invalid digit 2");
  assert(wordle_wordlist(buf, 6, "br-ad", "6", NULL) == 0 && "Missing char");
  assert(wordle_wordlist(buf, 6, "br-ad", "e0A9", NULL) == 0 &&
         "Missing char 2");
}

void test_examples(void) {
  size_t buf_size = 12;
  char buf[buf_size];

  size_t count = wordle_wordlist(buf, buf_size, "br---", "", "");
  assert(count == buf_size);
  assert(strcmp(wordle_get_nth_word(buf, count, 0), "braai") == 0);
  assert(strcmp(wordle_get_nth_word(buf, count, 1), "brace") == 0);
  assert(wordle_get_nth_word(buf, count, 2) == NULL);

  size_t buf_size2 = 33;
  char buf2[buf_size2];
  size_t count2 = wordle_wordlist(buf2, buf_size2, "-ield", "plz", "");
  assert(count2 == 30);
  assert(strcmp(wordle_get_nth_word(buf2, count2, 0), "bield") == 0);
  assert(strcmp(wordle_get_nth_word(buf2, count2, 1), "field") == 0);
  assert(strcmp(wordle_get_nth_word(buf2, count2, 2), "sield") == 0);
  assert(strcmp(wordle_get_nth_word(buf2, count2, 3), "wield") == 0);
  assert(strcmp(wordle_get_nth_word(buf2, count2, 4), "yield") == 0);
  assert(wordle_get_nth_word(buf2, count2, 5) == NULL);
}

size_t test_performance(void) {
  size_t buf_size = 100;
  char buf[buf_size];
  size_t accum = 0;
  for (int i = 0; i < 100000; ++i) {
    size_t count = wordle_wordlist(buf, buf_size, "-ield", "plz", "");
    accum += count;
  }
  return accum;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    test_nth_word_invalid_args();
    test_wordlist_invalid_args();
    test_examples();
  } else if (strcmp(argv[1], "-p") == 0) {
    test_performance();
  }
}
