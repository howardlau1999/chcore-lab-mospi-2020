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

#include <common/kprint.h>
#include <common/macro.h>
#include <common/uart.h>
#include <common/machine.h>

ALIGN(STACK_ALIGNMENT)
char kernel_stack[PLAT_CPU_NUM][KERNEL_STACK_SIZE];

int stack_backtrace();

// Test the stack backtrace function (lab 1 only)
__attribute__ ((optimize("O1")))
void stack_test(long x)
{
	kinfo("entering stack_test %d\n", x);
	if (x > 0)
		stack_test(x - 1);
	else
		stack_backtrace();
	kinfo("leaving stack_test %d\n", x);
}

void main(void *addr)
{
	/* Init uart */
	uart_init();
	kinfo("[ChCore] uart init finished\n");

	kinfo("Address of main() is 0x%lx\n", main);
	kinfo("123456 decimal is 0%o octal\n", 123456);

	stack_test(5);

	break_point();
	return;

	/* Should provide panic and use here */
	BUG("[FATAL] Should never be here!\n");
}
