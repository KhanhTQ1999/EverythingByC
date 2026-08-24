#include <stdio.h>
#include <stdlib.h>
#include "inc/types.h"
#include "inc/core/shell.h"

int main(int argc, char *argv[]) {
  struct shell_ctx ctx;
  shell_context_init(&ctx);

  i32 ret = shell_run(&ctx);
  if(ret != 0) {
      fprintf(stderr, "Shell exited with error code: %d\n", ret);
  }

  shell_context_free(&ctx);
  return ret;
}
