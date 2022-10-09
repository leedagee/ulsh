#pragma once
#define BUILTIN_NODE_NUM 2048
#define BUILTIN_ADD_COMMAND(NAME) builtin_put_entry(#NAME, builtin_##NAME)
#define BUILTIN_DECLARE(NAME) int builtin_##NAME(int argc, char* argv[])

typedef int (*BUILTIN_HANDLER)(int, char*[]);

struct builtin_trie_node {
  struct builtin_trie_node* head;
  struct builtin_trie_node* next;
  BUILTIN_HANDLER handler;
  char c;
};

static struct builtin_trie_node builtin_trie_root;

struct builtin_trie_node* builtin_new_node();
void builtin_put_entry(const char* name, BUILTIN_HANDLER handler);
BUILTIN_HANDLER builtin_find_entry(const char* name);
void builtin_init();
