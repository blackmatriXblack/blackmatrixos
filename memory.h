#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Page flags */
#define PAGE_PRESENT    0x01
#define PAGE_WRITE      0x02
#define PAGE_USER       0x04
#define PAGE_NOCACHE    0x08
#define PAGE_ACCESSED   0x20
#define PAGE_DIRTY      0x40

#define PAGE_SIZE       4096
#define PAGE_SHIFT      12
#define KERNEL_BASE     0x100000
#define HEAP_START      0x400000
#define HEAP_END        0x800000
#define STACK_TOP       0x90000

/* Page directory and table entry types */
typedef uint32_t page_dir_entry_t;
typedef uint32_t page_table_entry_t;

/* Memory map entry from BIOS */
typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
} PACKED mmap_entry_t;

/* Heap block header */
typedef struct heap_block {
    uint32_t size;
    bool     free;
    struct heap_block* next;
    struct heap_block* prev;
} heap_block_t;

/* Page frame allocator */
void     pmm_init(uint32_t mem_size_kb);
void     pmm_init_from_mmap(mmap_entry_t* entries, uint32_t count);
void*    pmm_alloc_frame(void);
void     pmm_free_frame(void* addr);
uint32_t pmm_get_free_frames(void);
uint32_t pmm_get_total_frames(void);

/* Paging */
void     paging_init(void);
void     paging_map(uint32_t virt, uint32_t phys, uint32_t flags);
void     paging_unmap(uint32_t virt);
uint32_t paging_get_physical(uint32_t virt);
void     paging_switch_directory(uint32_t* dir);

/* Heap allocator (kmalloc/kfree) */
void     heap_init(void);
void*    kmalloc(size_t size);
void*    kmalloc_aligned(size_t size, size_t alignment);
void*    kmalloc_phys(size_t size, uint32_t* phys);
void*    krealloc(void* ptr, size_t size);
void     kfree(void* ptr);
size_t   heap_get_used(void);
size_t   heap_get_free(void);

/* Utility */
void     memory_copy(void* dst, const void* src, uint32_t n);
void     memory_set(void* dst, uint8_t val, uint32_t n);

#endif
