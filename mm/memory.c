#include "../include/memory.h"
#include "../include/string.h"
#include "../include/vga.h"

/* ==============================================================================
 * Page Frame Manager (Physical Memory Manager)
 * ============================================================================== */

#define MAX_FRAMES  0x10000  /* 256MB worth of 4KB frames */
#define BITMAP_SIZE (MAX_FRAMES / 32)

static uint32_t frame_bitmap[BITMAP_SIZE];
static uint32_t total_frames = 0;
static uint32_t used_frames = 0;

static inline void frame_set(uint32_t frame) {
    frame_bitmap[frame / 32] |= (1 << (frame % 32));
}

static inline void frame_clear(uint32_t frame) {
    frame_bitmap[frame / 32] &= ~(1 << (frame % 32));
}

static inline bool frame_test(uint32_t frame) {
    return frame_bitmap[frame / 32] & (1 << (frame % 32));
}

void pmm_init(uint32_t mem_size_kb) {
    total_frames = mem_size_kb / 4;
    if (total_frames > MAX_FRAMES) total_frames = MAX_FRAMES;
    used_frames = 0;
    memset(frame_bitmap, 0, sizeof(frame_bitmap));

    /* Mark first 4MB as used (kernel + VGA + etc) */
    for (uint32_t i = 0; i < 1024; i++) {
        frame_set(i);
        used_frames++;
    }
}

void pmm_init_from_mmap(mmap_entry_t* entries, uint32_t count) {
    memset(frame_bitmap, 0xFF, sizeof(frame_bitmap)); /* Mark all as used */
    total_frames = 0;
    used_frames = 0;

    for (uint32_t i = 0; i < count; i++) {
        if (entries[i].type == 1) {  /* Usable memory */
            uint64_t base = entries[i].base;
            uint64_t len  = entries[i].length;
            uint32_t start_frame = (uint32_t)(base / 4096);
            uint32_t num_frames  = (uint32_t)(len / 4096);

            for (uint32_t f = start_frame; f < start_frame + num_frames && f < MAX_FRAMES; f++) {
                frame_clear(f);
                total_frames++;
            }
        }
    }

    /* Reserve first 4MB for kernel */
    for (uint32_t i = 0; i < 1024; i++) {
        if (!frame_test(i)) {
            frame_set(i);
            used_frames++;
        }
    }
}

void* pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < total_frames / 32; i++) {
        if (frame_bitmap[i] != 0xFFFFFFFF) {
            for (int j = 0; j < 32; j++) {
                if (!(frame_bitmap[i] & (1 << j))) {
                    uint32_t frame = i * 32 + j;
                    frame_set(frame);
                    used_frames++;
                    return (void*)(frame * 4096);
                }
            }
        }
    }
    return NULL;  /* Out of memory */
}

void pmm_free_frame(void* addr) {
    uint32_t frame = (uint32_t)addr / 4096;
    if (frame_test(frame)) {
        frame_clear(frame);
        used_frames--;
    }
}

uint32_t pmm_get_free_frames(void) {
    return total_frames - used_frames;
}

uint32_t pmm_get_total_frames(void) {
    return total_frames;
}

/* ==============================================================================
 * Virtual Memory (Paging)
 * ============================================================================== */

static uint32_t* page_directory = NULL;
static uint32_t* page_tables = NULL;

#define PAGE_DIR_INDEX(virt) ((virt) >> 22)
#define PAGE_TABLE_INDEX(virt) (((virt) >> 12) & 0x3FF)

void paging_init(void) {
    /* Allocate page directory (4KB aligned) */
    page_directory = (uint32_t*)pmm_alloc_frame();
    memset(page_directory, 0, 4096);

    /* Allocate space for first 4 page tables (16MB identity mapped) */
    page_tables = (uint32_t*)pmm_alloc_frame();

    /* Identity map first 16MB */
    for (uint32_t i = 0; i < 4096; i++) {
        page_tables[i] = (i * 4096) | PAGE_PRESENT | PAGE_WRITE;
    }

    /* Set up first 4 page directory entries */
    for (int i = 0; i < 4; i++) {
        page_directory[i] = ((uint32_t)page_tables + i * 4096) | PAGE_PRESENT | PAGE_WRITE;
    }

    /* Load page directory and enable paging */
    paging_switch_directory(page_directory);
}

void paging_map(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t dir_idx = PAGE_DIR_INDEX(virt);
    uint32_t tbl_idx = PAGE_TABLE_INDEX(virt);

    /* Get or create page table */
    if (!(page_directory[dir_idx] & PAGE_PRESENT)) {
        uint32_t* new_table = (uint32_t*)pmm_alloc_frame();
        memset(new_table, 0, 4096);
        page_directory[dir_idx] = (uint32_t)new_table | PAGE_PRESENT | PAGE_WRITE | flags;
    }

    uint32_t* table = (uint32_t*)(page_directory[dir_idx] & ~0xFFF);
    table[tbl_idx] = (phys & ~0xFFF) | PAGE_PRESENT | flags;

    /* Flush TLB for this page */
    __asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
}

void paging_unmap(uint32_t virt) {
    uint32_t dir_idx = PAGE_DIR_INDEX(virt);
    uint32_t tbl_idx = PAGE_TABLE_INDEX(virt);

    if (page_directory[dir_idx] & PAGE_PRESENT) {
        uint32_t* table = (uint32_t*)(page_directory[dir_idx] & ~0xFFF);
        table[tbl_idx] = 0;
        __asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
    }
}

uint32_t paging_get_physical(uint32_t virt) {
    uint32_t dir_idx = PAGE_DIR_INDEX(virt);
    uint32_t tbl_idx = PAGE_TABLE_INDEX(virt);

    if (!(page_directory[dir_idx] & PAGE_PRESENT)) return 0;
    uint32_t* table = (uint32_t*)(page_directory[dir_idx] & ~0xFFF);
    if (!(table[tbl_idx] & PAGE_PRESENT)) return 0;
    return (table[tbl_idx] & ~0xFFF) | (virt & 0xFFF);
}

void paging_switch_directory(uint32_t* dir) {
    page_directory = dir;
    __asm__ volatile (
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        : : "r"(dir) : "eax"
    );
}

/* ==============================================================================
 * Heap Allocator (First-fit with coalescing)
 * ============================================================================== */

static heap_block_t* heap_head = NULL;
static size_t heap_used = 0;
static size_t heap_total = 0;

void heap_init(void) {
    heap_head = (heap_block_t*)HEAP_START;
    heap_head->size = HEAP_END - HEAP_START - sizeof(heap_block_t);
    heap_head->free = true;
    heap_head->next = NULL;
    heap_head->prev = NULL;
    heap_total = HEAP_END - HEAP_START;
    heap_used = 0;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    size = ALIGN_UP(size, 4);

    heap_block_t* block = heap_head;
    while (block) {
        if (block->free && block->size >= size) {
            /* Split block if large enough */
            if (block->size > size + sizeof(heap_block_t) + 16) {
                heap_block_t* new_block = (heap_block_t*)((uint8_t*)block + sizeof(heap_block_t) + size);
                new_block->size = block->size - size - sizeof(heap_block_t);
                new_block->free = true;
                new_block->next = block->next;
                new_block->prev = block;
                if (block->next) block->next->prev = new_block;
                block->next = new_block;
                block->size = size;
            }
            block->free = false;
            heap_used += block->size;
            return (void*)((uint8_t*)block + sizeof(heap_block_t));
        }
        block = block->next;
    }
    return NULL;  /* Out of heap memory */
}

void* kmalloc_aligned(size_t size, size_t alignment) {
    void* ptr = kmalloc(size + alignment + sizeof(void*));
    if (!ptr) return NULL;

    uint32_t addr = (uint32_t)ptr + sizeof(void*);
    uint32_t aligned = ALIGN_UP(addr, alignment);
    ((void**)aligned)[-1] = ptr;
    return (void*)aligned;
}

void* kmalloc_phys(size_t size, uint32_t* phys) {
    void* addr = kmalloc(size);
    if (phys && addr) {
        *phys = paging_get_physical((uint32_t)addr);
    }
    return addr;
}

void* krealloc(void* ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) { kfree(ptr); return NULL; }

    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    if (block->size >= size) return ptr;

    void* new_ptr = kmalloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        kfree(ptr);
    }
    return new_ptr;
}

static void heap_merge_free(heap_block_t* block) {
    /* Merge with next */
    while (block->next && block->next->free) {
        block->size += sizeof(heap_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    /* Merge with prev */
    while (block->prev && block->prev->free) {
        block->prev->size += sizeof(heap_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
        block = block->prev;
    }
}

void kfree(void* ptr) {
    if (!ptr) return;

    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    block->free = true;
    heap_used -= block->size;
    heap_merge_free(block);
}

size_t heap_get_used(void) { return heap_used; }
size_t heap_get_free(void) { return heap_total - heap_used; }

/* Legacy aliases */
void memory_copy(void* dst, const void* src, uint32_t n) {
    memcpy(dst, src, n);
}

void memory_set(void* dst, uint8_t val, uint32_t n) {
    memset(dst, val, n);
}
