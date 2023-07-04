#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#include "curvefs/sdk/libcurvefs/libcurvefs.h"

int main(int argc, char** argv) {
    exact_args(argc, 1);

    char* fsname = get_filesystem_name();
    uintptr_t instance = curvefs_create();
    int rc = curvefs_mount(instance, fsname, "/");
    if (rc != 0) {
        fprintf(stderr, "mount failed: retcode = %d\n", rc);
        return rc;
    }

    rc = curvefs_mkdir(instance, argv[1], 0755);
    if (rc != 0) {
        fprintf(stderr, "mkdir failed: retcode = %d\n", rc);
        return rc;
    }

    return 0;
}
