#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char*
get_filesystem_name() {
    char* name = getenv("CURVE_FILESYSTEM_NAME");
    if (strlen(name) == 0) {
        exit(1);
    }
    return name;
}

void
exact_args(int argc, int number) {
    if (--argc == number) {
        return;
    }

    fprintf(stderr, "requires exactly %d argument[s]\n", number);
    exit(1);
}
