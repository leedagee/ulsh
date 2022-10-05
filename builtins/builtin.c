#include "builtin.h"

#include <stdlib.h>

#include "dirops.h"

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
  BUILTIN_ADD_COMMAND(cd);
  BUILTIN_ADD_COMMAND(pwd);
}