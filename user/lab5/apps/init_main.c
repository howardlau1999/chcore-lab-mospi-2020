#include "init.h"

#include <print.h>
#include <launcher.h>
#include <syscall.h>

// get a character from standard input
char getch()
{
	char c;
	// TODO: your code here

	return c;
}

int main(int argc, char *argv[])
{
	char *buf;
	int ret = 0;

	printf("init loads cpio image.\n");
	usys_fs_load_cpio(CPIO_BIN);

	boot_fs();

	while (1) {
		printf("\n");
		buf = readline("$ ");
		if (buf == NULL)
			usys_exit(0);
		if (buf[0] == 0)
			continue;
		if (builtin_cmd(buf))
			continue;
		if ((ret = run_cmd(buf)) < 0) {
			printf("Cannot run %s, ERROR %d\n", buf, ret);
		}
	}

	return 0;
}
