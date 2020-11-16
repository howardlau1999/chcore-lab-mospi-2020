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
#include <common/smp.h>
#include <common/kprint.h>
#include <common/machine.h>
#include <common/kmalloc.h>
#include <common/list.h>
#include <common/util.h>
#include <process/thread.h>
#include <common/macro.h>
#include <common/errno.h>
#include <process/thread.h>
#include <exception/irq.h>
#include <sched/context.h>

/* in arch/sched/idle.S */
void idle_thread_routine(void);

/*
 * rr_ready_queue
 * Per-CPU ready queue for ready tasks.
 */
struct list_head rr_ready_queue[PLAT_CPU_NUM];

/*
 * RR policy also has idle threads.
 * When no active user threads in ready queue,
 * we will choose the idle thread to execute.
 * Idle thread will **NOT** be in the RQ.
 */
struct thread idle_threads[PLAT_CPU_NUM];

/*
 * Lab4
 * Sched_enqueue
 * Put `thread` at the end of ready queue of assigned `affinity`.
 * If affinity = NO_AFF, assign the core to the current cpu.
 * If the thread is IDEL thread, do nothing!
 * Do not forget to check if the affinity is valid!
 */
int rr_sched_enqueue(struct thread *thread)
{
	if (thread == NULL || thread->thread_ctx == NULL || thread->thread_ctx->state == TS_READY) {
		return -EINVAL;
	}

	if (thread->thread_ctx->type == TYPE_IDLE) {
		return 0;
	}

	s32 affinity = thread->thread_ctx->affinity;
	if (affinity == NO_AFF) {
		affinity = smp_get_cpu_id();
	}

	if (affinity >= PLAT_CPU_NUM) {
		return -EINVAL;
	}

	thread->thread_ctx->state = TS_READY;
	thread->thread_ctx->cpuid = affinity;
	list_append(&thread->ready_queue_node, &rr_ready_queue[affinity]);
	return 0;
}

/*
 * Lab4
 * Sched_dequeue
 * remove `thread` from its current residual ready queue
 * Do not forget to add some basic parameter checking
 */
int rr_sched_dequeue(struct thread *thread)
{
	if (thread == NULL || thread->thread_ctx == NULL
		|| thread->thread_ctx->type == TYPE_IDLE
		|| thread->thread_ctx->state != TS_READY) {
		return -EINVAL;
	}

	thread->thread_ctx->state = TS_INTER;
	list_del(&thread->ready_queue_node);
	return 0;
}

/*
 * Lab4
 * The helper function
 * Choose an appropriate thread and dequeue from ready queue
 * 
 * If there is no ready thread in the current CPU's ready queue, 
 * choose the idle thread of the CPU.
 * 
 * Do not forget to check the type and 
 * state of the chosen thread
 */
struct thread *rr_sched_choose_thread(void)
{
	u32 cpuid = smp_get_cpu_id();
	if (list_empty(&rr_ready_queue[cpuid])) {
		return &idle_threads[cpuid];
	}

	struct thread *target = list_entry(rr_ready_queue[cpuid].next, struct thread, ready_queue_node);

	BUG_ON(rr_sched_dequeue(target));
	return target;
}

static inline void rr_sched_refill_budget(struct thread *target, u32 budget)
{
}

/*
 * Lab4
 * Schedule a thread to execute.
 * This function will suspend current running thread, if any, and schedule
 * another thread from `rr_ready_queue[cpu_id]`.
 * 
 * Hints:
 * Macro DEFAULT_BUDGET defines the value for resetting thread's budget.
 * After you get one thread from rr_sched_choose_thread, pass it to
 * switch_to_thread() to prepare for switch_context().
 * Then ChCore can call eret_to_thread() to return to user mode.
 */
int rr_sched(void)
{
	struct thread* target = rr_sched_choose_thread();
	BUG_ON(target == NULL);
	if (current_thread != NULL) {
		current_thread->thread_ctx->state = TS_INTER;
		rr_sched_enqueue(current_thread);
	}
	return switch_to_thread(target);
}

/*
 * Initialize the per thread queue and idle thread.
 */
int rr_sched_init(void)
{
	int i = 0;

	/* Initialize global variables */
	for (i = 0; i < PLAT_CPU_NUM; i++) {
		current_threads[i] = NULL;
		init_list_head(&rr_ready_queue[i]);
	}

	/* Initialize one idle thread for each core and insert into the RQ */
	for (i = 0; i < PLAT_CPU_NUM; i++) {
		/* Set the thread context of the idle threads */
		BUG_ON(!(idle_threads[i].thread_ctx = create_thread_ctx()));
		/* We will set the stack and func ptr in arch_idle_ctx_init */
		init_thread_ctx(&idle_threads[i], 0, 0, MIN_PRIO, TYPE_IDLE, i);
		/* Call arch-dependent function to fill the context of the idle
		 * threads */
		arch_idle_ctx_init(idle_threads[i].thread_ctx,
				   idle_thread_routine);
		/* Idle thread is kernel thread which do not have vmspace */
		idle_threads[i].vmspace = NULL;
	}
	kdebug("Scheduler initialized. Create %d idle threads.\n", i);

	return 0;
}

/*
 * Lab4
 * Handler called each time a timer interrupt is handled
 * Do not forget to call sched_handle_timer_irq() in proper code location.
 */
void rr_sched_handle_timer_irq(void)
{
}

struct sched_ops rr = {
	.sched_init = rr_sched_init,
	.sched = rr_sched,
	.sched_enqueue = rr_sched_enqueue,
	.sched_dequeue = rr_sched_dequeue,
	.sched_choose_thread = rr_sched_choose_thread,
	.sched_handle_timer_irq = rr_sched_handle_timer_irq,
};
