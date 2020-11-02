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

/* Scheduler related functions are implemented here */
#include <sched/sched.h>
#include <common/kprint.h>
#include <common/machine.h>
#include <common/kmalloc.h>
#include <common/list.h>
#include <common/util.h>
#include <process/thread.h>
#include <common/macro.h>
#include <common/errno.h>
#include <process/thread.h>
#include <exception/exception.h>
#include <sched/context.h>

struct thread *current_threads[PLAT_CPU_NUM];

char thread_type[][TYPE_STR_LEN] = {
	"IDLE  ",
	"ROOT  ",
	"USER  ",
	"SHADOW",
	"KERNEL",
	"TESTS "
};

char thread_state[][STATE_STR_LEN] = {
	"TS_INIT      ",
	"TS_READY     ",
	"TS_INTER     ",
	"TS_RUNNING   ",
	"TS_EXIT      ",
	"TS_WAITING   ",
	"TS_EXITING   "
};

/*
 * Switch vmspace and arch-related stuff
 * Return the context pointer which should be set to stack pointer register
 */
u64 switch_context(void)
{
	struct thread *target_thread;
	struct thread_ctx *target_ctx;

	target_thread = current_thread;
	BUG_ON(!target_thread);
	BUG_ON(!target_thread->thread_ctx);

	target_ctx = target_thread->thread_ctx;

	/* These 3 types of thread do not have vmspace */
	if (target_thread->thread_ctx->type != TYPE_IDLE &&
	    target_thread->thread_ctx->type != TYPE_KERNEL &&
	    target_thread->thread_ctx->type != TYPE_TESTS) {

		BUG_ON(!target_thread->vmspace);
		/*
		 * Recording the CPU the thread runs on: for TLB maintainence.
		 * switch_context is always required for running a (new) thread.
		 * So, we invoke record_running_cpu here.
		 */
		BUG_ON(!target_thread->vmspace);
		switch_thread_vmspace_to(target_thread);
	}
	/*
	 * Lab3: Your code here
	 * Return the correct value in order to make eret_to_thread work correctly
	 * in main.c
	 */
	return 0;
}
