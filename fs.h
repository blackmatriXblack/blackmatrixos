#ifndef FS_H
#define FS_H

#include "types.h"

#define FS_MAX_NAME      256
#define FS_MAX_PATH      1024
#define FS_MAX_FILES     512
#define FS_MAX_OPEN      32
#define FS_BLOCK_SIZE    512
#define FS_MAX_BLOCKS    4096

typedef enum {
    FS_FILE = 0,
    FS_DIRECTORY,
    FS_SYMLINK,
    FS_DEVICE,
    FS_PIPE,
} fs_type_t;

typedef struct inode {
    char          name[FS_MAX_NAME];
    fs_type_t     type;
    uint32_t      size;
    uint32_t      permissions;
    uint32_t      uid;
    uint32_t      gid;
    uint32_t      created_time;
    uint32_t      modified_time;
    uint32_t      accessed_time;
    uint32_t      blocks[12];     /* Direct blocks */
    uint32_t      indirect;       /* Single indirect block */
    uint32_t      double_indirect;/* Double indirect block */
    uint32_t      ref_count;
    bool          dirty;
    struct inode* parent;
    struct inode* child;          /* First child (if directory) */
    struct inode* sibling;        /* Next sibling */
} inode_t;

typedef struct {
    inode_t* inode;
    uint32_t position;
    uint32_t flags;
    bool     in_use;
} file_descriptor_t;

typedef struct {
    char     name[FS_MAX_NAME];
    uint32_t inode_num;
    fs_type_t type;
} dirent_t;

typedef struct {
    inode_t*     inode;
    uint32_t     index;
} dir_t;

/* File system operations */
void     fs_init(void);
int      fs_mount(void);
int      fs_unmount(void);

/* File operations */
int      fs_open(const char* path, uint32_t flags);
int      fs_close(int fd);
int      fs_read(int fd, void* buf, uint32_t size);
int      fs_write(int fd, const void* buf, uint32_t size);
int      fs_seek(int fd, int offset, int whence);
int      fs_tell(int fd);
int      fs_truncate(int fd, uint32_t size);

/* Directory operations */
int      fs_mkdir(const char* path, uint32_t permissions);
int      fs_rmdir(const char* path);
int      fs_unlink(const char* path);
dir_t*   fs_opendir(const char* path);
dirent_t* fs_readdir(dir_t* dir);
int      fs_closedir(dir_t* dir);
int      fs_ls(const char* path);

/* Link operations */
int      fs_link(const char* oldpath, const char* newpath);
int      fs_symlink(const char* target, const char* linkpath);

/* Stat */
typedef struct {
    fs_type_t type;
    uint32_t  size;
    uint32_t  permissions;
    uint32_t  uid;
    uint32_t  gid;
    uint32_t  created_time;
    uint32_t  modified_time;
    uint32_t  nlinks;
} fs_stat_t;

int fs_stat(const char* path, fs_stat_t* st);
int fs_fstat(int fd, fs_stat_t* st);

/* Utility */
bool     fs_exists(const char* path);
char*    fs_getcwd(void);
int      fs_chdir(const char* path);
char*    fs_normalize_path(const char* path);

/* Flags */
#define O_RDONLY   0x0001
#define O_WRONLY   0x0002
#define O_RDWR     0x0003
#define O_CREAT    0x0004
#define O_TRUNC    0x0008
#define O_APPEND   0x0010
#define O_EXCL     0x0020

#define SEEK_SET   0
#define SEEK_CUR   1
#define SEEK_END   2

#define FS_PERM_READ   0x04
#define FS_PERM_WRITE  0x02
#define FS_PERM_EXEC   0x01

#endif
