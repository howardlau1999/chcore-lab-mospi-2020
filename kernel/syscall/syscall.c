/*
 * Copyright (c) 2020 Institute of Parallel And Distributed Systems (IPADS), Shanghai Jiao Tong University (SJTU)
 * OS-Lab-2020 (i.e., ChCore) is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *   http://license.coscl.org.cn/MulanPSL
 *   THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 *   PURPOSE.
 *   See the Mulan PSL v1 for more details.
 */

#include <common/types.h>
#include <common/uart.h>
#include <common/uaccess.h>
#include <common/kmalloc.h>
#include <common/mm.h>
#include <common/kprint.h>
#include "syscall_num.h"

void sys_debug(long arg)
{
	kinfo("[syscall] sys_debug: %lx\n", arg);
}

void sys_putc(char ch)
{
	/*
	 * Lab3: Your code here
	 * Send ch to the screen in anyway as your like
	 */
}

/*
 * Lab3: Your code here
 * Update the syscall table as you like to redirect syscalls
 * to functions accordingly
 */
const void *syscall_table[NR_SYSCALL] = {
	[0 ... NR_SYSCALL - 1] = sys_debug,
	/* lab3 syscalls finished */
};
