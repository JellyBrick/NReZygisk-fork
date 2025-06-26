#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>

#include "utils.h"
#include "mount.h"
#include "daemon.h"

static int mount_make_ns() {
    int base_ns = open("/proc/self/ns/mnt", O_RDONLY | O_CLOEXEC);
    if (base_ns == -1) {
        PLOGE("mount_make_ns: base_ns = open(/proc/self/ns/mnt)");
        return -1;
    }

    if (unshare(CLONE_NEWNS) == -1) {
        PLOGE("mount_make_ns: unshare(CLONE_NEWNS)");
        return -1;
    }

    int new_ns = open("/proc/self/ns/mnt", O_RDONLY | O_CLOEXEC);
    if (new_ns == -1) {
        PLOGE("mount_make_ns: new_ns = open(/proc/self/ns/mnt)");
    }

    setns(base_ns, CLONE_NEWNS);
    close(base_ns);

    return new_ns;
}

static void mount_save_ns(const char *save, int ns_fd) {
    if (ns_fd < 0) return;
    char path[64];
    int pid = getpid();
    snprintf(path, sizeof(path), "/proc/%d/fd/%d", pid, ns_fd);

    unlink(save);
    symlink(path, save);
}

void mount_ns_main(char **argv) {
    if (access(TMP_PATH "/clean_zygote", F_OK) != 0) {
        return;
    }

    int ns64;
    if (LP_SELECT(false, true)) {
        ns64 = mount_make_ns();
    }
    int ns32 = mount_make_ns();

    int ready_pipe[2] = {-1, -1};
    pipe(ready_pipe);

    if (fork() == 0) {
        close(ready_pipe[0]);
        strncpy(argv[0], "zygisk-mnt", strlen(argv[0]));

        if (LP_SELECT(false, true)) {
            mount_save_ns(TMP_PATH "/mns64", ns64);
        }
        mount_save_ns(TMP_PATH "/mns32", ns32);

        close(ready_pipe[1]);
        while (pause());
    }

    close(ready_pipe[1]);
    char dummy;
    TEMP_FAILURE_RETRY(read(ready_pipe[0], &dummy, 1));
}
