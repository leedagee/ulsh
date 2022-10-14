#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../parser.h"

int main() {
  for (;;) {
    char *l = readline("> ");
    if (l == NULL) {
      puts("");
      return 0;
    }
    add_history(l);

    struct parse_result_t *res, *cur;
    int len = parse_command(l, &res);

    printf("Length=%d\n", len);

    cur = res;
    while (cur != NULL) {
      printf("argc=%d, flags=%d, f_in=%s, f_out=%s\n", cur->argc, cur->flags,
             cur->f_in, cur->f_out);
      for (int i = 0; i < cur->argc; i++) {
        printf("\targv[%d](%d)=%s\n", i, strlen(cur->argv[i]), cur->argv[i]);
      }
      cur = cur->next;
    }

    free_parse_result(res);

    free(l);
  }
}
