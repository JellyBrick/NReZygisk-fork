#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/mount.h>

#include "utils.h"
#include "mount.h"
#include "daemon.h"

static bool mount_make_ns() {
    if (unshare(CLONE_NEWNS) == -1) {
        PLOGE("mount_make_ns: unshare(CLONE_NEWNS)");
        return false;
    }

    mount(0, "/", 0, MS_REC | MS_SLAVE, 0);
    return true;
}

static void mount_save_ns(const char *save) {
    char path[64];
    int pid = getpid();
    snprintf(path, sizeof(path), "/proc/%d/ns/mnt", pid);

    unlink(save);
    symlink(path, save);
}

static void mount_spawn_ns(char **argv, const char *pname, const char *save) {
    int ready_pipe[2] = {-1, -1};
    pipe(ready_pipe);

    if (fork() == 0) {
        close(ready_pipe[0]);
        if (argv && argv[0] && strlen(argv[0]) >= strlen(pname)) {
            strcpy(argv[0], pname);
        }
        prctl(PR_SET_NAME, pname);

        if (!mount_make_ns()) {
            _exit(0);
        }

        mount_save_ns(save);

        close(ready_pipe[1]);
        while (pause());
    }

    close(ready_pipe[1]);
    char dummy;
    TEMP_FAILURE_RETRY(read(ready_pipe[0], &dummy, 1));
}

void mount_ns_main(char **argv) {
    if (access(TMP_PATH "/clean_zygote", F_OK) != 0) {
        return;
    }

    if (LP_SELECT(false, true)) {
        mount_spawn_ns(argv, "zygisk-m64\0\0\0\0\0\0", TMP_PATH "/mns64");
    }

    mount_spawn_ns(argv, "zygisk-m32\0\0\0\0\0\0", TMP_PATH "/mns32");
}
