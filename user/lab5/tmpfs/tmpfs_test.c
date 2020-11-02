#include <syscall.h>
#include <launcher.h>
#include <malloc.h>
#include <string.h>
#include "tmpfs_server.h"

#define TEST_FUNC(name) \
  do { \
    if (name() == 0) { \
      printf(#name" pass!\n"); \
    } else { \
      printf(#name" fail!\n"); \
    } \
  } while (0)

static int test_fs_server_init()
{
	return fs_server_init(CPIO_BIN);
}

static int test_scan()
{
	int count = 4096;
	char *buf = malloc(count);
	char *str = malloc(256);
	int start;
	int ret;
	void *vp;
	struct dirent *p;
	int i;
	start = 0;
	do {
		ret = fs_server_scan("/tar", start, buf, count);
		vp = buf;
		start += ret;
		for (i = 0; i < ret; i++) {
			p = vp;
			strcpy(str, p->d_name);
			printf("%s\n", str);
			vp += p->d_reclen;
		}
	} while (ret != 0);
	printf("/tar has %d files\n", start);
	start = 0;
	do {
		ret = fs_server_scan("/tar/test", start, buf, count);
		vp = buf;
		start += ret;
		// printf("/tar/test ret = %d\n", ret);
		for (i = 0; i < ret; i++) {
			p = vp;
			strcpy(str, p->d_name);
			printf("%s\n", str);
			vp += p->d_reclen;
		}
	} while (ret != 0);
	printf("/tar/test has %d files\n", start);

	return 0;
}

static int test_mkdir()
{
	int err = fs_server_mkdir("/dir");
	return err;
}

static int test_creat()
{
	int err = fs_server_creat("/file");
	return err;
}

static int test_rmdir()
{
	int err = fs_server_rmdir("/dir");
	return err;
}

static int test_unlink()
{
	int err = fs_server_unlink("/file");
	return err;
}

#define FILE_LEN (PAGE_SIZE * 2)

static int test_write_read()
{
	char *buf[2];
	buf[0] = malloc(FILE_LEN);
	buf[1] = malloc(FILE_LEN);
	int ret = fs_server_write("/file", 0, buf[0], FILE_LEN);
	if (ret != FILE_LEN) {
		printf("write ret=%x len=%x\n", ret, FILE_LEN);
		return -1;
	}
	ret = fs_server_read("/file", 0, buf[1], FILE_LEN);
	if (ret != FILE_LEN) {
		printf("read ret=%x len=%x\n", ret, FILE_LEN);
		return -1;
	}
	return memcmp(buf[0], buf[1], FILE_LEN);
}

static int test_deepfile()
{
	int err;

	err = fs_server_mkdir("/dir/dir2");
	if (err != 0)
		return err;

	err = fs_server_creat("/dir/dir2/file");
	if (err != 0)
		return err;

	err = fs_server_rmdir("/dir/dir2");
	if (err != -ENOTEMPTY)
		return -1;
	err = fs_server_unlink("/dir/dir2/file");
	if (err != 0)
		return err;

	err = fs_server_rmdir("/dir");
	if (err != -ENOTEMPTY)
		return -1;
	err = fs_server_rmdir("/dir/dir2");
	if (err != 0)
		return err;
	err = fs_server_rmdir("/dir");
	if (err != 0)
		return err;

	return 0;
}

int main()
{
	TEST_FUNC(test_fs_server_init);
	TEST_FUNC(test_scan);
	TEST_FUNC(test_mkdir);
	TEST_FUNC(test_creat);
	TEST_FUNC(test_write_read);

	TEST_FUNC(test_deepfile);

	TEST_FUNC(test_unlink);
	TEST_FUNC(test_rmdir);

	return 0;
}
