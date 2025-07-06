#ifndef SYSCALLS_H
#define SYSCALLS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sys/socket.h>
#include <sys/syscall.h>
#include <errno.h>

#if defined(__x86_64__)

/*
 * INFO: x86-64 ABI:
 * Syscall number: rax
 * Arg 1: rdi
 * Arg 2: rsi
 * Arg 3: rdx
 * Arg 4: r10
 * Arg 5: r8
 * Arg 6: r9
 * Return: rax
 * Instruction: syscall
 * Clobbers: rcx, r11
 */

static inline long raw_syscall0(long n) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n)
        : "memory", "rcx", "r11"
    );
    return ret;
}

static inline long raw_syscall1(long n, long a1) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1)
        : "memory", "rcx", "r11"
    );
    return ret;
}

static inline long raw_syscall2(long n, long a1, long a2) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2)
        : "memory", "rcx", "r11"
    );
    return ret;
}

static inline long raw_syscall3(long n, long a1, long a2, long a3) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3)
        : "memory", "rcx", "r11"
    );
    return ret;
}

static inline long raw_syscall4(long n, long a1, long a2, long a3, long a4) {
    long ret;
    register long r10 __asm__("r10") = a4;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
        : "memory", "rcx", "r11"
    );
    return ret;
}

static inline long raw_syscall5(long n, long a1, long a2, long a3, long a4, long a5) {
    long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
        : "memory", "rcx", "r11"
    );
    return ret;
}

static inline long raw_syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6) {
    long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    register long r9 __asm__("r9") = a6;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9)
        : "memory", "rcx", "r11"
    );
    return ret;
}

#elif defined(__i386__)

/*
 * INFO: i386 ABI:
 * Syscall number: eax
 * Arg 1: ebx
 * Arg 2: ecx
 * Arg 3: edx
 * Arg 4: esi
 * Arg 5: edi
 * Arg 6: ebp
 * Return: eax
 * Instruction: int $0x80
 */

static inline long raw_syscall0(long n) {
    long ret;
    __asm__ volatile (
            "int $0x80"
            : "=a"(ret)
            : "a"(n)
            : "memory"
            );
    return ret;
}

static inline long raw_syscall1(long n, long a1) {
    long ret;
    __asm__ volatile (
            "int $0x80"
            : "=a"(ret)
            : "a"(n), "b"(a1)
            : "memory"
            );
    return ret;
}

static inline long raw_syscall2(long n, long a1, long a2) {
    long ret;
    __asm__ volatile (
            "int $0x80"
            : "=a"(ret)
            : "a"(n), "b"(a1), "c"(a2)
            : "memory"
            );
    return ret;
}

static inline long raw_syscall3(long n, long a1, long a2, long a3) {
    long ret;
    __asm__ volatile (
            "int $0x80"
            : "=a"(ret)
            : "a"(n), "b"(a1), "c"(a2), "d"(a3)
            : "memory"
            );
    return ret;
}

static inline long raw_syscall4(long n, long a1, long a2, long a3, long a4) {
    long ret;
    __asm__ volatile (
            "int $0x80"
            : "=a"(ret)
            : "a"(n), "b"(a1), "c"(a2), "d"(a3), "S"(a4)
            : "memory"
            );
    return ret;
}

static inline long raw_syscall5(long n, long a1, long a2, long a3, long a4, long a5) {
    long ret;
    __asm__ volatile (
            "int $0x80"
            : "=a"(ret)
            : "a"(n), "b"(a1), "c"(a2), "d"(a3), "S"(a4), "D"(a5)
            : "memory"
            );
    return ret;
}

static inline long raw_syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6) {
    long ret;
    // ebp is used for arg6, we must preserve it.
    __asm__ volatile (
            "pushl %%ebp\n\t"
            "movl %7, %%ebp\n\t"
            "int $0x80\n\t"
            "popl %%ebp"
            : "=a"(ret)
            : "a"(n), "b"(a1), "c"(a2), "d"(a3), "S"(a4), "D"(a5), "g"(a6)
            : "memory"
            );
    return ret;
}


#elif defined(__aarch64__)

/*
 * INFO: AArch64 ABI:
 * Syscall number: x8
 * Arg 1-6: x0-x5
 * Return: x0
 * Instruction: svc #0
 */

#define _RAW_SYSCALL_BODY(N, A1, A2, A3, A4, A5, A6) \
    long ret; \
    register long x8 __asm__("x8") = (N); \
    register long x0 __asm__("x0") = (long)(A1); \
    register long x1 __asm__("x1") = (long)(A2); \
    register long x2 __asm__("x2") = (long)(A3); \
    register long x3 __asm__("x3") = (long)(A4); \
    register long x4 __asm__("x4") = (long)(A5); \
    register long x5 __asm__("x5") = (long)(A6); \
    __asm__ volatile ( \
        "svc #0" \
        : "=r"(x0) \
        : "r"(x8), "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5) \
        : "memory", "cc" \
    ); \
    ret = x0; \
    return ret;

static inline long raw_syscall0(long n) { _RAW_SYSCALL_BODY(n, 0, 0, 0, 0, 0, 0); }
static inline long raw_syscall1(long n, long a1) { _RAW_SYSCALL_BODY(n, a1, 0, 0, 0, 0, 0); }
static inline long raw_syscall2(long n, long a1, long a2) { _RAW_SYSCALL_BODY(n, a1, a2, 0, 0, 0, 0); }
static inline long raw_syscall3(long n, long a1, long a2, long a3) { _RAW_SYSCALL_BODY(n, a1, a2, a3, 0, 0, 0); }
static inline long raw_syscall4(long n, long a1, long a2, long a3, long a4) { _RAW_SYSCALL_BODY(n, a1, a2, a3, a4, 0, 0); }
static inline long raw_syscall5(long n, long a1, long a2, long a3, long a4, long a5) { _RAW_SYSCALL_BODY(n, a1, a2, a3, a4, a5, 0); }
static inline long raw_syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6) { _RAW_SYSCALL_BODY(n, a1, a2, a3, a4, a5, a6); }

#elif defined(__arm__)

/*
 * INFO: ARM EABI:
 * Syscall number: r7
 * Arg 1-6: r0-r5
 * Return: r0
 * Instruction: svc #0
 */

#define _RAW_SYSCALL_BODY(N, A1, A2, A3, A4, A5, A6) \
    long ret; \
    register long r7 __asm__("r7") = (N); \
    register long r0 __asm__("r0") = (long)(A1); \
    register long r1 __asm__("r1") = (long)(A2); \
    register long r2 __asm__("r2") = (long)(A3); \
    register long r3 __asm__("r3") = (long)(A4); \
    register long r4 __asm__("r4") = (long)(A5); \
    register long r5 __asm__("r5") = (long)(A6); \
    __asm__ volatile ( \
        "svc #0" \
        : "=r"(r0) \
        : "r"(r7), "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5) \
        : "memory", "cc" \
    ); \
    ret = r0; \
    return ret;

static inline long raw_syscall0(long n) { _RAW_SYSCALL_BODY(n, 0, 0, 0, 0, 0, 0); }
static inline long raw_syscall1(long n, long a1) { _RAW_SYSCALL_BODY(n, a1, 0, 0, 0, 0, 0); }
static inline long raw_syscall2(long n, long a1, long a2) { _RAW_SYSCALL_BODY(n, a1, a2, 0, 0, 0, 0); }
static inline long raw_syscall3(long n, long a1, long a2, long a3) { _RAW_SYSCALL_BODY(n, a1, a2, a3, 0, 0, 0); }
static inline long raw_syscall4(long n, long a1, long a2, long a3, long a4) { _RAW_SYSCALL_BODY(n, a1, a2, a3, a4, 0, 0); }
static inline long raw_syscall5(long n, long a1, long a2, long a3, long a4, long a5) { _RAW_SYSCALL_BODY(n, a1, a2, a3, a4, a5, 0); }
static inline long raw_syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6) { _RAW_SYSCALL_BODY(n, a1, a2, a3, a4, a5, a6); }

#else

#error "Unsupported architecture for raw syscalls."

#endif

#define RAW_TEMP_FAILURE_RETRY(exp) ({     \
    __typeof__(exp) _rc;                   \
    do {                                   \
        _rc = (exp);                       \
    } while (_rc == -EINTR);               \
    _rc; })

static void *raw_memcpy(void *dest, const void *src, unsigned long n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    for (unsigned long i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

static ssize_t raw_read_n(int fd, void *buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t r = RAW_TEMP_FAILURE_RETRY(raw_syscall3(__NR_read, fd, (long)buf + total, n - total));
        if (r <= 0) return r;
        total += r;
    }
    return (ssize_t) total;
}


static ssize_t raw_write_n(int fd, const void *buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t w = RAW_TEMP_FAILURE_RETRY(raw_syscall3(__NR_write, fd, (long)buf + total, n - total));
        if (w <= 0) return w;
        total += w;
    }
    return (ssize_t) total;
}


static long raw_send_fd(int socket, int fd_to_send) {
    struct msghdr msg = {};
    char buf[CMSG_SPACE(sizeof(fd_to_send))] = {0};

    char s_fd[] = "FD\0";
    struct iovec io = {.iov_base = s_fd, .iov_len = 2};

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd_to_send));

    raw_memcpy(CMSG_DATA(cmsg), &fd_to_send, sizeof(fd_to_send));

    return RAW_TEMP_FAILURE_RETRY(raw_syscall3(__NR_sendmsg, socket, (long) &msg, 0));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SYSCALLS_H */