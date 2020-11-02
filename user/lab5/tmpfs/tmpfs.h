#pragma once

#include <type.h>
#include <ipc.h>

#include "fs_defs.h"
#include "hashtable.h"
#include "radix.h"

#define PREFIX "[tmpfs]"

#define info(fmt, ...) printf(PREFIX " " fmt, ## __VA_ARGS__)
#define error(fmt, ...) printf(PREFIX " " fmt, ## __VA_ARGS__)

#define MAX_FILENAME_LEN (255)

#define FS_REG (1)
#define FS_DIR (2)

struct string {
	char *str;
	size_t len;
	u64 hash;
};

// dentry is linked by `hlist_node`
// we can get the address of this structure by address of `node` using 
// `container_of_safe`
struct dentry {
	struct string name;
	struct inode *inode;
	struct hlist_node node;
};

struct inode {
	u64 type;
	size_t size;
	union {
		struct htable dentries;
		struct radix data;
	};
};

int init_tmpfs(void);

int tfs_creat(struct inode *dir, const char *name, size_t len);
int tfs_mkdir(struct inode *dir, const char *name, size_t len);

int tfs_namex(struct inode **dirat, const char **name, int mkdir_p);
int tfs_remove(struct inode *dir, const char *name, size_t len);

ssize_t tfs_file_read(struct inode *inode, off_t offset, char *buff,
		      size_t size);
ssize_t tfs_file_write(struct inode *inode, off_t offset, const char *data,
		       size_t size);

int tfs_scan(struct inode *dir, unsigned int start, void *buf, void *end);
struct inode *tfs_open_path(const char *path);

int tfs_load_image(const char *start);
