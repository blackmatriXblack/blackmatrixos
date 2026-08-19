#include "../include/process.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../include/stdio.h"
#include "../include/vga.h"
#include "../include/timer.h"
#include "../include/io.h"

static process_t process_table[MAX_PROCESSES];
static process_t* current_process = NULL;
static pid_t next_pid = 1;
static int process_count = 0;

/* Idle process - runs when no other process is ready */
UNUSED static void idle_process(void) {
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void scheduler_init(void) {
    memset(process_table, 0, sizeof(process_table));
    current_process = NULL;
    next_pid = 1;
    process_count = 0;

    /* Create kernel (idle) process as PID 0 */
    process_t* idle = &process_table[0];
    idle->pid = 0;
    strcpy(idle->name, "idle");
    idle->state = PROC_READY;
    idle->priority = 0;
    idle->kernel_stack = (uint32_t)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
    idle->time_slice = 1;
    idle->ticks = 0;
    current_process = idle;
    process_count = 1;
}

pid_t process_create(const char* name, void (*entry)(void), int priority) {
    /* Find empty slot */
    int slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROC_UNUSED) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return -1;

    process_t* proc = &process_table[slot];
    memset(proc, 0, sizeof(process_t));

    proc->pid = next_pid++;
    strncpy(proc->name, name, PROC_NAME_LEN - 1);
    proc->state = PROC_READY;
    proc->priority = priority;
    proc->parent_pid = current_process ? current_process->pid : 0;

    /* Allocate kernel stack */
    proc->kernel_stack = (uint32_t)kmalloc(KERNEL_STACK_SIZE);
    if (!proc->kernel_stack) return -1;
    proc->kernel_stack += KERNEL_STACK_SIZE;

    /* Set up initial context */
    proc->context.eip = (uint32_t)entry;
    proc->context.esp = proc->kernel_stack;
    proc->context.ebp = proc->kernel_stack;
    proc->context.eflags = 0x202;  /* IF set */
    proc->context.cs = 0x08;       /* Kernel code segment */
    proc->context.ds = 0x10;
    proc->context.es = 0x10;
    proc->context.fs = 0x10;
    proc->context.gs = 0x10;
    proc->context.ss = 0x10;
    proc->context.eax = 0;
    proc->context.ebx = 0;
    proc->context.ecx = 0;
    proc->context.edx = 0;
    proc->context.esi = 0;
    proc->context.edi = 0;

    proc->time_slice = 5;  /* 5 timer ticks per process */
    proc->ticks = 0;
    proc->priority = priority;

    process_count++;
    return proc->pid;
}

void process_exit(int code) {
    if (!current_process || current_process->pid == 0) return;
    current_process->state = PROC_ZOMBIE;
    current_process->exit_code = code;
    process_count--;
    schedule();
}

void process_yield(void) {
    schedule();
}

void process_sleep(uint32_t ms) {
    current_process->sleep_until = timer_get_ticks() + (ms * TICK_HZ / 1000);
    current_process->state = PROC_SLEEPING;
    schedule();
}

void process_wake(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && process_table[i].state == PROC_SLEEPING) {
            process_table[i].state = PROC_READY;
            break;
        }
    }
}

void schedule(void) {
    if (!current_process) return;

    /* Wake sleeping processes */
    uint32_t now = timer_get_ticks();
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROC_SLEEPING && process_table[i].sleep_until <= now) {
            process_table[i].state = PROC_READY;
        }
    }

    /* Round-robin with priority */
    process_t* prev = current_process;
    process_t* next = NULL;

    /* Search for next ready process starting from current */
    int start = (prev->pid + 1) % MAX_PROCESSES;
    int best_priority = -1;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        int idx = (start + i) % MAX_PROCESSES;
        if (process_table[idx].state == PROC_READY) {
            if (process_table[idx].priority > best_priority) {
                best_priority = process_table[idx].priority;
                next = &process_table[idx];
            }
        }
    }

    if (!next) {
        /* No ready process found, stay with current or idle */
        if (prev->state == PROC_RUNNING) return;
        next = &process_table[0]; /* Idle process */
    }

    if (next == prev && prev->state == PROC_RUNNING) return;

    /* Switch process */
    if (prev->state == PROC_RUNNING) prev->state = PROC_READY;
    next->state = PROC_RUNNING;

    process_t* old = current_process;
    current_process = next;

    /* Context switch */
    context_switch(&old->context, &next->context);
}

process_t* process_get_current(void) {
    return current_process;
}

process_t* process_get(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) return &process_table[i];
    }
    return NULL;
}

int process_get_count(void) {
    return process_count;
}

void process_kill(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) {
            process_table[i].state = PROC_KILLED;
            if (process_table[i].kernel_stack) {
                kfree((void*)(process_table[i].kernel_stack - KERNEL_STACK_SIZE));
            }
            process_table[i].state = PROC_UNUSED;
            process_count--;
            break;
        }
    }
}

const char* process_state_str(proc_state_t state) {
    switch (state) {
        case PROC_UNUSED:    return "UNUSED";
        case PROC_CREATED:   return "CREATED";
        case PROC_READY:     return "READY";
        case PROC_RUNNING:   return "RUNNING";
        case PROC_SLEEPING:  return "SLEEP";
        case PROC_WAITING:   return "WAIT";
        case PROC_ZOMBIE:    return "ZOMBIE";
        case PROC_KILLED:    return "KILLED";
        default:             return "UNKNOWN";
    }
}

void process_list(void) {
    kprintf("  PID  STATE      PRI  TICKS  NAME\n");
    kprintf("  ---  ---------  ---  -----  ----\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROC_UNUSED) {
            kprintf("  %3d  %9s  %3d  %5d  %s\n",
                    process_table[i].pid,
                    process_state_str(process_table[i].state),
                    process_table[i].priority,
                    process_table[i].ticks,
                    process_table[i].name);
        }
    }
    kprintf("Total processes: %d\n", process_count);
}
