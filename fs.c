#include "../include/fs.h"
#include "../include/stdio.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../include/vga.h"

/* ==============================================================================
 * In-Memory File System (RAMFS)
 * ============================================================================== */

static inode_t inode_pool[FS_MAX_FILES];
int inode_count = 0;
static inode_t* root_inode = NULL;
static char cwd_path[FS_MAX_PATH] = "/";
static file_descriptor_t fd_table[FS_MAX_OPEN];
static uint32_t current_time = 0;

/* Block storage (simulated) */
static uint8_t block_storage[FS_MAX_BLOCKS][FS_BLOCK_SIZE];
static bool block_used[FS_MAX_BLOCKS];

static inode_t* alloc_inode(void) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (inode_pool[i].ref_count == 0 && inode_pool[i].name[0] == '\0') {
            memset(&inode_pool[i], 0, sizeof(inode_t));
            inode_pool[i].ref_count = 1;
            inode_count++;
            return &inode_pool[i];
        }
    }
    return NULL;
}

static void free_inode(inode_t* inode) {
    if (!inode) return;
    /* Free blocks */
    for (int i = 0; i < 12; i++) {
        if (inode->blocks[i]) {
            block_used[inode->blocks[i]] = false;
            inode->blocks[i] = 0;
        }
    }
    if (inode->indirect) block_used[inode->indirect] = false;
    memset(inode->name, 0, FS_MAX_NAME);
    inode->ref_count = 0;
    inode_count--;
}

static uint32_t alloc_block(void) {
    for (uint32_t i = 0; i < FS_MAX_BLOCKS; i++) {
        if (!block_used[i]) {
            block_used[i] = true;
            memset(block_storage[i], 0, FS_BLOCK_SIZE);
            return i;
        }
    }
    return 0;
}

static inode_t* find_child(inode_t* parent, const char* name) {
    if (!parent || parent->type != FS_DIRECTORY) return NULL;
    inode_t* child = parent->child;
    while (child) {
        if (strcmp(child->name, name) == 0) return child;
        child = child->sibling;
    }
    return NULL;
}

static void add_child(inode_t* parent, inode_t* child) {
    child->parent = parent;
    child->sibling = parent->child;
    parent->child = child;
}

static void remove_child(inode_t* parent, inode_t* child) {
    if (!parent || !parent->child) return;
    if (parent->child == child) {
        parent->child = child->sibling;
        child->sibling = NULL;
        return;
    }
    inode_t* prev = parent->child;
    while (prev->sibling) {
        if (prev->sibling == child) {
            prev->sibling = child->sibling;
            child->sibling = NULL;
            return;
        }
        prev = prev->sibling;
    }
}

static inode_t* resolve_path(const char* path) {
    if (!path || !path[0]) return root_inode;

    inode_t* current;
    const char* p;

    if (path[0] == '/') {
        current = root_inode;
        p = path + 1;
    } else {
        current = resolve_path(cwd_path);
        p = path;
    }

    char component[FS_MAX_NAME];
    while (*p) {
        /* Skip slashes */
        while (*p == '/') p++;
        if (!*p) break;

        /* Extract component */
        int len = 0;
        while (*p && *p != '/' && len < FS_MAX_NAME - 1) {
            component[len++] = *p++;
        }
        component[len] = '\0';

        if (strcmp(component, ".") == 0) continue;
        if (strcmp(component, "..") == 0) {
            if (current->parent) current = current->parent;
            continue;
        }

        inode_t* child = find_child(current, component);
        if (!child) return NULL;
        current = child;
    }
    return current;
}

static void get_path(inode_t* node, char* buf, int size) {
    if (!node || !buf || size <= 0) return;

    char temp[FS_MAX_PATH];
    int pos = FS_MAX_PATH - 1;
    temp[pos] = '\0';

    while (node && node != root_inode) {
        int len = strlen(node->name);
        pos -= len;
        if (pos < 1) break;
        memcpy(&temp[pos], node->name, len);
        pos--;
        temp[pos] = '/';
        node = node->parent;
    }

    if (pos == FS_MAX_PATH - 1) {
        temp[--pos] = '/';
    }

    int copy_len = FS_MAX_PATH - pos;
    if (copy_len > size) copy_len = size;
    strncpy(buf, &temp[pos], copy_len - 1);
}

/* ==============================================================================
 * Public API
 * ============================================================================== */

void fs_init(void) {
    memset(inode_pool, 0, sizeof(inode_pool));
    memset(fd_table, 0, sizeof(fd_table));
    memset(block_used, 0, sizeof(block_used));
    inode_count = 0;
    current_time = 0;

    /* Create root directory */
    root_inode = alloc_inode();
    strcpy(root_inode->name, "/");
    root_inode->type = FS_DIRECTORY;
    root_inode->permissions = 0755;
    root_inode->parent = root_inode;  /* Root's parent is itself */

    /* Create standard directories */
    fs_mkdir("/bin", 0755);
    fs_mkdir("/etc", 0755);
    fs_mkdir("/home", 0755);
    fs_mkdir("/tmp", 0777);
    fs_mkdir("/usr", 0755);
    fs_mkdir("/var", 0755);
    fs_mkdir("/dev", 0755);
    fs_mkdir("/home/user", 0755);

    /* Create some default files */
    int fd = fs_open("/etc/passwd", O_CREAT | O_WRONLY);
    const char* passwd = "root:x:0:0:root:/root:/bin/sh\nuser:x:1000:1000:User:/home/user:/bin/sh\n";
    fs_write(fd, passwd, strlen(passwd));
    fs_close(fd);

    fd = fs_open("/etc/hostname", O_CREAT | O_WRONLY);
    const char* hostname = "cosmos\n";
    fs_write(fd, hostname, strlen(hostname));
    fs_close(fd);

    fd = fs_open("/etc/motd", O_CREAT | O_WRONLY);
    const char* motd =
        "\n"
        "  ██████╗ ██████╗ ███████╗███╗   ███╗ ██████╗ ███████╗\n"
        "  ██╔════╝██╔═══██╗██╔════╝████╗ ████║██╔═══██╗██╔════╝\n"
        "  ██║     ██║   ██║███████╗██╔████╔██║██║   ██║███████╗\n"
        "  ██║     ██║   ██║╚════██║██║╚██╔╝██║██║   ██║╚════██║\n"
        "  ╚██████╗╚██████╔╝███████║██║ ╚═╝ ██║╚██████╔╝███████║\n"
        "   ╚═════╝ ╚═════╝ ╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝\n"
        "\n  Welcome to CosmosOS - A tiny operating system\n"
        "  Type 'help' for available commands.\n\n";
    fs_write(fd, motd, strlen(motd));
    fs_close(fd);

    fd = fs_open("/home/user/readme.txt", O_CREAT | O_WRONLY);
    const char* readme = "Welcome to CosmosOS!\n\n"
        "This is a hobby operating system written in C and x86 assembly.\n"
        "Features:\n"
        "  - Preemptive multitasking\n"
        "  - Virtual memory with paging\n"
        "  - In-memory file system\n"
        "  - Interactive shell\n"
        "  - Simple GUI (type 'gui' to start)\n"
        "  - Basic network stack\n\n"
        "Type 'help' for a list of commands.\n";
    fs_write(fd, readme, strlen(readme));
    fs_close(fd);

    fd = fs_open("/home/user/hello.c", O_CREAT | O_WRONLY);
    const char* hello = "#include <stdio.h>\n\nint main(void) {\n    printf(\"Hello, CosmosOS!\\n\");\n    return 0;\n}\n";
    fs_write(fd, hello, strlen(hello));
    fs_close(fd);
}

int fs_open(const char* path, uint32_t flags) {
    inode_t* inode = resolve_path(path);

    if (!inode && (flags & O_CREAT)) {
        /* Create new file */
        /* Extract parent directory and filename */
        char dir_path[FS_MAX_PATH];
        char filename[FS_MAX_NAME];
        strncpy(dir_path, path, FS_MAX_PATH - 1);
        char* last_slash = strrchr(dir_path, '/');
        if (last_slash && last_slash != dir_path) {
            strncpy(filename, last_slash + 1, FS_MAX_NAME - 1);
            *last_slash = '\0';
        } else if (last_slash == dir_path) {
            strncpy(filename, path + 1, FS_MAX_NAME - 1);
            strcpy(dir_path, "/");
        } else {
            strncpy(filename, path, FS_MAX_NAME - 1);
            strcpy(dir_path, cwd_path);
        }

        inode_t* parent = resolve_path(dir_path);
        if (!parent || parent->type != FS_DIRECTORY) return -1;

        inode = alloc_inode();
        if (!inode) return -1;
        strncpy(inode->name, filename, FS_MAX_NAME - 1);
        inode->type = FS_FILE;
        inode->permissions = 0644;
        add_child(parent, inode);
    }

    if (!inode) return -1;

    if (flags & O_TRUNC) {
        inode->size = 0;
        for (int i = 0; i < 12; i++) {
            if (inode->blocks[i]) {
                block_used[inode->blocks[i]] = false;
                inode->blocks[i] = 0;
            }
        }
    }

    /* Find free fd */
    for (int i = 0; i < FS_MAX_OPEN; i++) {
        if (!fd_table[i].in_use) {
            fd_table[i].inode = inode;
            fd_table[i].position = (flags & O_APPEND) ? inode->size : 0;
            fd_table[i].flags = flags;
            fd_table[i].in_use = true;
            inode->ref_count++;
            return i;
        }
    }
    return -1;
}

int fs_close(int fd) {
    if (fd < 0 || fd >= FS_MAX_OPEN || !fd_table[fd].in_use) return -1;
    fd_table[fd].inode->ref_count--;
    fd_table[fd].in_use = false;
    return 0;
}

int fs_read(int fd, void* buf, uint32_t size) {
    if (fd < 0 || fd >= FS_MAX_OPEN || !fd_table[fd].in_use) return -1;
    file_descriptor_t* f = &fd_table[fd];
    inode_t* inode = f->inode;

    if (f->position >= inode->size) return 0;
    if (f->position + size > inode->size) size = inode->size - f->position;

    uint32_t remaining = size;
    uint8_t* dest = (uint8_t*)buf;
    uint32_t pos = f->position;

    while (remaining > 0) {
        uint32_t block_idx = pos / FS_BLOCK_SIZE;
        uint32_t block_off = pos % FS_BLOCK_SIZE;
        uint32_t chunk = FS_BLOCK_SIZE - block_off;
        if (chunk > remaining) chunk = remaining;

        if (block_idx < 12 && inode->blocks[block_idx]) {
            memcpy(dest, &block_storage[inode->blocks[block_idx]][block_off], chunk);
        } else {
            memset(dest, 0, chunk);
        }

        dest += chunk;
        pos += chunk;
        remaining -= chunk;
    }

    f->position += size;
    return size;
}

int fs_write(int fd, const void* buf, uint32_t size) {
    if (fd < 0 || fd >= FS_MAX_OPEN || !fd_table[fd].in_use) return -1;
    file_descriptor_t* f = &fd_table[fd];
    inode_t* inode = f->inode;

    uint32_t remaining = size;
    const uint8_t* src = (const uint8_t*)buf;
    uint32_t pos = f->position;

    while (remaining > 0) {
        uint32_t block_idx = pos / FS_BLOCK_SIZE;
        uint32_t block_off = pos % FS_BLOCK_SIZE;
        uint32_t chunk = FS_BLOCK_SIZE - block_off;
        if (chunk > remaining) chunk = remaining;

        if (block_idx >= 12) {
            /* Too many blocks needed */
            break;
        }

        if (!inode->blocks[block_idx]) {
            inode->blocks[block_idx] = alloc_block();
            if (!inode->blocks[block_idx]) break;
        }

        memcpy(&block_storage[inode->blocks[block_idx]][block_off], src, chunk);
        src += chunk;
        pos += chunk;
        remaining -= chunk;
    }

    f->position = pos;
    if (pos > inode->size) inode->size = pos;
    return size - remaining;
}

int fs_seek(int fd, int offset, int whence) {
    if (fd < 0 || fd >= FS_MAX_OPEN || !fd_table[fd].in_use) return -1;
    file_descriptor_t* f = &fd_table[fd];
    switch (whence) {
        case SEEK_SET: f->position = offset; break;
        case SEEK_CUR: f->position += offset; break;
        case SEEK_END: f->position = f->inode->size + offset; break;
        default: return -1;
    }
    return f->position;
}

int fs_tell(int fd) {
    if (fd < 0 || fd >= FS_MAX_OPEN || !fd_table[fd].in_use) return -1;
    return fd_table[fd].position;
}

int fs_mkdir(const char* path, uint32_t permissions) {
    char dir_path[FS_MAX_PATH];
    char dirname[FS_MAX_NAME];
    strncpy(dir_path, path, FS_MAX_PATH - 1);
    char* last_slash = strrchr(dir_path, '/');
    if (last_slash && last_slash != dir_path) {
        strncpy(dirname, last_slash + 1, FS_MAX_NAME - 1);
        *last_slash = '\0';
    } else if (last_slash == dir_path) {
        strncpy(dirname, path + 1, FS_MAX_NAME - 1);
        strcpy(dir_path, "/");
    } else {
        strncpy(dirname, path, FS_MAX_NAME - 1);
        strcpy(dir_path, cwd_path);
    }

    inode_t* parent = resolve_path(dir_path);
    if (!parent || parent->type != FS_DIRECTORY) return -1;
    if (find_child(parent, dirname)) return -1;

    inode_t* dir = alloc_inode();
    if (!dir) return -1;
    strncpy(dir->name, dirname, FS_MAX_NAME - 1);
    dir->type = FS_DIRECTORY;
    dir->permissions = permissions;
    add_child(parent, dir);
    return 0;
}

int fs_unlink(const char* path) {
    inode_t* inode = resolve_path(path);
    if (!inode || inode->type == FS_DIRECTORY) return -1;
    if (inode->ref_count > 0) return -1;
    remove_child(inode->parent, inode);
    free_inode(inode);
    return 0;
}

int fs_rmdir(const char* path) {
    inode_t* inode = resolve_path(path);
    if (!inode || inode->type != FS_DIRECTORY) return -1;
    if (inode->child) return -1;  /* Directory not empty */
    remove_child(inode->parent, inode);
    free_inode(inode);
    return 0;
}

dir_t* fs_opendir(const char* path) {
    inode_t* inode = resolve_path(path);
    if (!inode || inode->type != FS_DIRECTORY) return NULL;
    dir_t* dir = (dir_t*)kmalloc(sizeof(dir_t));
    dir->inode = inode;
    dir->index = 0;
    return dir;
}

dirent_t* fs_readdir(dir_t* dir) {
    if (!dir || !dir->inode) return NULL;
    inode_t* child = dir->inode->child;
    for (uint32_t i = 0; i < dir->index && child; i++) {
        child = child->sibling;
    }
    if (!child) return NULL;

    static dirent_t entry;
    strncpy(entry.name, child->name, FS_MAX_NAME - 1);
    entry.type = child->type;
    entry.inode_num = 0;  /* Simplified */
    dir->index++;
    return &entry;
}

int fs_closedir(dir_t* dir) {
    if (dir) kfree(dir);
    return 0;
}

int fs_ls(const char* path) {
    inode_t* inode = resolve_path(path ? path : cwd_path);
    if (!inode || inode->type != FS_DIRECTORY) return -1;

    inode_t* child = inode->child;
    while (child) {
        const char* type_str = (child->type == FS_DIRECTORY) ? "DIR " : "FILE";
        kprintf("  %s  %8d  %s\n", type_str, child->size, child->name);
        child = child->sibling;
    }
    return 0;
}

int fs_stat(const char* path, fs_stat_t* st) {
    inode_t* inode = resolve_path(path);
    if (!inode || !st) return -1;
    st->type = inode->type;
    st->size = inode->size;
    st->permissions = inode->permissions;
    st->uid = inode->uid;
    st->gid = inode->gid;
    st->created_time = inode->created_time;
    st->modified_time = inode->modified_time;
    st->nlinks = 1;
    return 0;
}

int fs_fstat(int fd, fs_stat_t* st) {
    if (fd < 0 || fd >= FS_MAX_OPEN || !fd_table[fd].in_use || !st) return -1;
    inode_t* inode = fd_table[fd].inode;
    st->type = inode->type;
    st->size = inode->size;
    st->permissions = inode->permissions;
    st->uid = inode->uid;
    st->gid = inode->gid;
    st->created_time = inode->created_time;
    st->modified_time = inode->modified_time;
    st->nlinks = 1;
    return 0;
}

bool fs_exists(const char* path) {
    return resolve_path(path) != NULL;
}

char* fs_getcwd(void) {
    return cwd_path;
}

int fs_chdir(const char* path) {
    inode_t* inode = resolve_path(path);
    if (!inode || inode->type != FS_DIRECTORY) return -1;
    get_path(inode, cwd_path, FS_MAX_PATH);
    return 0;
}

int fs_mount(void) { return 0; }
int fs_unmount(void) { return 0; }
int fs_truncate(int fd UNUSED, uint32_t size UNUSED) { return -1; }
int fs_link(const char* old UNUSED, const char* newp UNUSED) { return -1; }
int fs_symlink(const char* target UNUSED, const char* link UNUSED) { return -1; }
char* fs_normalize_path(const char* path) { return (char*)path; }
