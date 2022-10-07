#include <ncurses.h>
#include <stdlib.h>

#define BUFFER_SIZE 16384

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "%s takes exactly 1 argumentsn.\n", argv[0]);
    return 1;
  }
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("Cannot open file");
    return 1;
  }

  initscr();
  scrollok(stdscr, TRUE);
  noecho();

  char buf[16384];

  while (!feof(fp)) {
    size_t len = fread(buf, 1, BUFFER_SIZE, fp);
    addnstr(buf, len);
  }

  for (;;) {
    char c = getch();
    switch (c) {
      case 'q':
        endwin();
        exit(0);
      case 'j':
        scrl(1);
        break;
      case 'k':
        scrl(-1);
        break;
      default:
        break;
    }
    refresh();
  }

  endwin();
  return 0;
}