#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include "utils.h"

struct init_fork {
    pid_t pid;
    void *next_breakpoint;
    long old_data;
    struct user_regs_struct *restore_regs;
    int stop;
};

void attr_hook_fork_init(struct init_fork *proc);
void attr_hook_fork_free(struct init_fork *proc);
void attr_hook_fork_realloc(struct init_fork **procs, size_t old_len, size_t new_len);
void attr_hook_prepare(void);
void attr_hook_place_first_breakpoint(struct init_fork *proc);
bool attr_hook_handle(struct init_fork *proc);
void attr_hook_bad_status();

#endif /* BREAKPOINT_H */
