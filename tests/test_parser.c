#include "../parser.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

int main() {
  struct parse_result_t *comp = NULL;
  int len;

  len = parse_command(";", &comp);
  assert(len == 1);
  assert(comp == NULL);
  free_parse_result(comp);

  len = parse_command("cmd1;cmd2;", &comp);
  assert(comp != NULL);
  assert(len == 5);
  assert(comp->argc == 1);
  assert(strcmp(comp->argv[0], "cmd1") == 0);
  free_parse_result(comp);

  len = parse_command("cmd1 ; cmd2 ;", &comp);
  assert(comp != NULL);
  assert(len == 6);
  assert(comp->argc == 1);
  assert(strcmp(comp->argv[0], "cmd1") == 0);
  free_parse_result(comp);

  len = parse_command("cmd1&cmd2;", &comp);
  assert(comp != NULL);
  assert(len == 5);
  assert(comp->argc == 1);
  assert(comp->flags & PARSE_RESULT_BACKGROUND);
  free_parse_result(comp);

  len = parse_command("cmd1|cmd2;", &comp);
  assert(comp != NULL);
  assert(len == 10);
  assert(comp->next->argc == 1);
  free_parse_result(comp);

  len = parse_command("\"|&<>\"", &comp);
  assert(comp != NULL);
  assert(len == 6);
  free_parse_result(comp);
}
