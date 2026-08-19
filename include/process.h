#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES   64
#define PROC_NAME_LEN   32
#define KERNEL_STACK_SIZE 4096
#define USER_STACK_SIZE   8192

typedef enum {
    PROC_UNUSED = 0,
    PROC_CREATED,
    PROC_READY,
    PROC_RUNNING,
    PROC_SLEEPING,
    PROC_WAITING,
    PROC_ZOMBIE,
    PROC_KILLED
} proc_state_t;

typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    uint32_t eip, eflags;
    uint32_t cs, ds, es, fs, gs, ss;
} PACKED context_t;

typedef struct process {
    pid_t        pid;
    pid_t        parent_pid;
    char         name[PROC_NAME_LEN];
    proc_state_t state;
    context_t    context;
    uint32_t     kernel_stack;
    uint32_t     user_stack;
    uint32_t     page_directory;
    uint32_t     time_slice;
    uint32_t     ticks;
    int          exit_code;
    int          priority;
    uint32_t     sleep_until;
    struct process* next;
} process_t;

/* Scheduler */
void        scheduler_init(void);
pid_t       process_create(const char* name, void (*entry)(void), int priority);
void        process_exit(int code);
void        process_yield(void);
void        process_sleep(uint32_t ms);
void        process_wake(pid_t pid);
void        schedule(void);
process_t*  process_get_current(void);
process_t*  process_get(pid_t pid);
int         process_get_count(void);
void        process_list(void);
void        process_kill(pid_t pid);
const char* process_state_str(proc_state_t state);

/* Context switch (implemented in assembly) */
extern void context_switch(context_t* old_ctx, context_t* new_ctx);

#endif
