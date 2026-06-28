#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedef.h"
#include "shell.h"

int main(int argc, char *argv[]) {
    i32 rc = shell_run();
    return rc;
}