#include "builtin.h"

#include <stdlib.h>

#include "dirops.h"
#include "procops.h"
#include "jobops.h"

struct builtin_trie_node *builtin_new_node() {
  static struct builtin_trie_node nodes[BUILTIN_NODE_NUM];
  static int count = 0;
  return &nodes[count++];
}

void builtin_put_entry(const char *name, BUILTIN_HANDLER handler) {
  struct builtin_trie_node *now = &builtin_trie_root;
  for (const char *c = name; *c; c++) {
    struct builtin_trie_node **next = &now->head;
    while (*next != NULL && (*next)->c != *c) {
      next = &(*next)->next;
    }
    if (*next == NULL) {
      *next = builtin_new_node();
      (*next)->c = *c;
    }
    now = *next;
  }
  now->handler = handler;
}

BUILTIN_HANDLER builtin_find_entry(const char *name) {
  struct builtin_trie_node *now = &builtin_trie_root;
  for (const char *c = name; *c; c++) {
    struct builtin_trie_node *next;
    for (next = now->head; next != NULL && next->c != *c; next = next->next) {
    }
    if (next == NULL) return NULL;
    now = next;
  }
  return now->handler;  // nullable
}

void builtin_init() {
  // dirops
  BUILTIN_ADD_COMMAND(cd);
  BUILTIN_ADD_COMMAND(pwd);

  // procops
  BUILTIN_ADD_COMMAND(exec);
  BUILTIN_ADD_COMMAND(exit);
  BUILTIN_ADD_COMMAND(export);

  // jobops
  BUILTIN_ADD_COMMAND(jobs);
  BUILTIN_ADD_COMMAND(fg);
  BUILTIN_ADD_COMMAND(bg);
}
