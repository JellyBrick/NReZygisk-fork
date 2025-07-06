#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <asm-generic/fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/un.h>
#include <sys/socket.h>

#include "raw_syscalls.h"

/*
 * INFO: Things in this file will be ran in a state where this .so is not properly loaded.
 * Library functions must not be called, raw syscalls must be used instead.
 * Other essential things are also not working, like TLS or relocations.
 */


/*
 * INFO: visibility("default") must not be used, as it makes accesses go through the GOT,
 * which is not supported due to the aforementioned reasons of half-assed .so loading.
 */
#define EXPORT __attribute__((visibility("protected")))

#define DCL_HOOK_FUNC(ret, func, ...)         \
  EXPORT ret (*init_old_##func)(__VA_ARGS__); \
  EXPORT ret init_new_##func(__VA_ARGS__)

EXPORT volatile bool init_is_unhooked;

int init_sock;
struct sockaddr_un init_sock_addr = {
        .sun_family = AF_UNIX,
        .sun_path = "/data/adb/rezygisk/init_con.sock\0"
};

DCL_HOOK_FUNC(pid_t, fork) {
    if (init_is_unhooked || raw_syscall0(__NR_gettid) != 1) {
        return init_old_fork();
    }

    char dummy = 0;
    int pipefd[2];
    raw_syscall2(__NR_pipe2, (long) pipefd, O_CLOEXEC);

    pid_t new_pid = init_old_fork();

    if (new_pid == 0) {
        raw_syscall1(__NR_close, pipefd[1]);
        raw_read_n(pipefd[0], &dummy, 1);
        raw_syscall1(__NR_close, pipefd[0]);
    } else {
        raw_syscall1(__NR_close, pipefd[0]);
        raw_write_n(init_sock, &new_pid, sizeof(new_pid));
        raw_read_n(init_sock, &dummy, 1);
        raw_write_n(pipefd[1], &dummy, 1);
        raw_syscall1(__NR_close, pipefd[1]);
    }

    return new_pid;
}

EXPORT int init_entry() {
    int sockets[2];
    long r = raw_syscall4(__NR_socketpair, AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, (long) sockets);
    if (r < 0) {
        return 1000 - r;
    }

    long socket = raw_syscall3(__NR_socket, AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (socket < 0) {
        return 2000 - socket;
    }

    r = raw_syscall3(__NR_connect, socket, (long) &init_sock_addr, sizeof(init_sock_addr));
    if (r < 0) {
        return 3000 - r;
    }

    r = raw_send_fd(socket, sockets[1]);
    if (r < 0) {
        return 4000 - r;
    }

    raw_syscall1(__NR_close, sockets[1]);
    raw_syscall1(__NR_close, socket);

    init_is_unhooked = true;
    init_sock = sockets[0];
    init_old_fork = 0;

    return 42;
}