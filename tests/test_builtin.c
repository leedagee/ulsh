#include <assert.h>
#include <stdio.h>

#include "../builtins/builtin.h"
#define BODY(NAME) \
  {                \
    puts(#NAME);   \
    return 0;      \
  }
#define TESTFUNC(NAME) BUILTIN_DECLARE(NAME) BODY(NAME)

TESTFUNC(ls)
TESTFUNC(ld)
TESTFUNC(pgg)
TESTFUNC(pggg)

int main() {
  builtin_put_entry("ls", builtin_ls);
  builtin_put_entry("ld", builtin_ld);
  builtin_put_entry("pgg", builtin_pgg);
  builtin_put_entry("pggg", builtin_pggg);
  assert(builtin_find_entry("ls") == builtin_ls);
  builtin_find_entry("ls")(0, NULL);
  assert(builtin_find_entry("pgggg") == NULL);
  assert(builtin_find_entry("pg") == NULL);
}
