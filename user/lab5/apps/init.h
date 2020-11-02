#pragma once

void boot_fs(void);

// read a command from stdin leading by `prompt`
// put the commond in `buf` and return `buf`
char *readline(const char *prompt);

// run `ls`, `echo`, `cat`, `cd`, `top`
// return true if `cmdline` is a builtin command
int builtin_cmd(char *cmdline);

// run other command, such as execute an executable file
// return true if run sccessfully
int run_cmd(char *cmdline);
