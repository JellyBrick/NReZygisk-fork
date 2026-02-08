#pragma once

#include <jni.h>

extern void *start_addr;
extern size_t block_size;

void clean_mounts(char **argv, char **envp);

void hook_functions();

extern "C" void send_seccomp_event();
