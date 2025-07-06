#ifndef INIT_H
#define INIT_H

#include <stdbool.h>

extern bool init_injected;
extern bool init_hooked;
extern int init_sock;
void init_inject();
void init_suspend_hooks();
void init_resume_hooks();
void init_went_well();


#endif /* INIT_H */