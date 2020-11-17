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
#include <common/types.h>
#include <common/util.h>
#include <common/elf.h>
#include <common/kmalloc.h>
#include <common/mm.h>
#include <common/uaccess.h>
#include <process/thread.h>
#include <sched/context.h>
#include <common/registers.h>
#include <common/cpio.h>
#include <exception/exception.h>

#include "thread_env.h"

static
int thread_init(struct thread *thread, struct process *process,
		u64 stack, u64 pc, u32 prio, u32 type, s32 aff)
{
	thread->process = obj_get(process, PROCESS_OBJ_ID, TYPE_PROCESS);
	thread->vmspace = obj_get(process, VMSPACE_OBJ_ID, TYPE_VMSPACE);
	obj_put(thread->process);
	obj_put(thread->vmspace);
	/* Thread context is used as the kernel stack for that thread */
	thread->thread_ctx = create_thread_ctx();
	if (!thread->thread_ctx)
		return -ENOMEM;
	init_thread_ctx(thread, stack, pc, prio, type, aff);
	/* add to process */
	list_add(&thread->node, &process->thread_list);

	return 0;
}

/*
 * Thread can be freed in two ways:
 * 1. Free through cap. The thread can be in arbitary state except for TS_EXIT
 *    and TS_EXITING. If the state is TS_RUNNING, let the scheduler free it
 *    later as the context of thread is used when entering kernel.
 * 2. Free through sys_exit. The thread is TS_EXIT and can be directly freed.
 */
void thread_deinit(void *thread_ptr)
{
	struct thread *thread;
	struct object *object;
	struct process *process;
	bool exit_process = false;

	thread = thread_ptr;

	object = container_of(thread, struct object, opaque);
	object->refcount = 1;

	process = thread->process;
	list_del(&thread->node);
	if (list_empty(&process->thread_list))
		exit_process = true;

	destroy_thread_ctx(thread);

	if (exit_process)
		process_exit(process);
}

int thread_create(struct process *process, u64 stack, u64 pc, u64 arg, u32 prio,
		  u32 type, s32 aff)
{
	struct thread *thread;
	int cap, ret = 0;

	if (!process) {
		ret = -ECAPBILITY;
		goto out_fail;
	}

	thread = obj_alloc(TYPE_THREAD, sizeof(*thread));
	if (!thread) {
		ret = -ENOMEM;
		goto out_obj_put;
	}
	if ((ret =
	     thread_init(thread, process, stack, pc, prio, type, aff)) != 0)
		goto out_free_obj;
	/* We should provide a separate syscall to set the affinity */
	arch_set_thread_arg(thread, arg);
	/* cap is thread_cap in the target process */
	cap = cap_alloc(process, thread, 0);
	if (cap < 0) {
		ret = cap;
		goto out_free_obj;
	}
	/* ret is thread_cap in the current_process */
	cap = cap_copy(process, current_process, cap, 0, 0);

	/* TYPE_KERNEL => do nothing */
	return cap;

 out_free_obj:
	obj_free(thread);
 out_obj_put:
	obj_put(process);
 out_fail:
	return ret;
}

#define PFLAGS2VMRFLAGS(PF)                                                    \
	(((PF)&PF_X ? VMR_EXEC : 0) | ((PF)&PF_W ? VMR_WRITE : 0) |            \
	 ((PF)&PF_R ? VMR_READ : 0))

#define OFFSET_MASK (0xFFF)

/* load binary into some process (process) */
static u64 load_binary(struct process *process,
		       struct vmspace *vmspace,
		       const char *bin, struct process_metadata *metadata)
{
	struct elf_file *elf;
	vmr_prop_t flags;
	int i, r;
	size_t seg_sz, seg_map_sz;
	u64 p_vaddr;

	int *pmo_cap;
	struct pmobject *pmo;
	u64 ret;

	elf = elf_parse_file(bin);
	pmo_cap = kmalloc(elf->header.e_phnum * sizeof(*pmo_cap));
	if (!pmo_cap) {
		r = -ENOMEM;
		goto out_fail;
	}

	/* load each segment in the elf binary */
	for (i = 0; i < elf->header.e_phnum; ++i) {
		pmo_cap[i] = -1;
		if (elf->p_headers[i].p_type == PT_LOAD) {
			/*
			 * Lab3: Your code here
			 * prepare the arguments for the two following function calls: pmo_init
			 * and vmspace_map_range.
			 * pmo_init allocates the demanded size of physical memory for the PMO.
			 * vmspace_map_range maps the pmo to a sepcific virtual memory address.
			 * You should get the size of current segment and the virtual address
			 * to be mapped from elf headers.
			 * HINT: we suggest you use the seg_sz and p_vaddr variables for
			 * raw data extracted from the elf header and use seg_map_sz for the
			 * page aligned segment size. Take care of the page alignment when allocating
			 * and mapping physical memory.
			 */
			seg_sz = elf->p_headers[i].p_memsz;
			p_vaddr = elf->p_headers[i].p_vaddr;
			seg_map_sz = ROUND_UP(p_vaddr + seg_sz, PAGE_SIZE) - ROUND_DOWN(p_vaddr, PAGE_SIZE);

			pmo = obj_alloc(TYPE_PMO, sizeof(*pmo));
			if (!pmo) {
				r = -ENOMEM;
				goto out_free_cap;
			}
			pmo_init(pmo, PMO_DATA, seg_map_sz, 0);
			pmo_cap[i] = cap_alloc(process, pmo, 0);
			if (pmo_cap[i] < 0) {
				r = pmo_cap[i];
				goto out_free_obj;
			}

			/*
			 * Lab3: Your code here
			 * You should copy data from the elf into the physical memory in pmo.
			 * The physical address of a pmo can be get from pmo->start.
			 */

			memcpy((void *) ((phys_to_virt(pmo->start) + (p_vaddr - ROUND_DOWN(p_vaddr, PAGE_SIZE)))),
				bin + elf->p_headers[i].p_offset, elf->p_headers[i].p_filesz);
			flags = PFLAGS2VMRFLAGS(elf->p_headers[i].p_flags);

			ret = vmspace_map_range(vmspace,
						ROUND_DOWN(p_vaddr, PAGE_SIZE),
						seg_map_sz, flags, pmo);

			BUG_ON(ret != 0);
		}
	}

	/* return binary metadata */
	if (metadata != NULL) {
		metadata->phdr_addr = elf->p_headers[0].p_vaddr +
		    elf->header.e_phoff;
		metadata->phentsize = elf->header.e_phentsize;
		metadata->phnum = elf->header.e_phnum;
		metadata->flags = elf->header.e_flags;
		metadata->entry = elf->header.e_entry;
	}

	kfree((void *)bin);

	/* PC: the entry point */
	return elf->header.e_entry;
 out_free_obj:
	obj_free(pmo);
 out_free_cap:
	for (--i; i >= 0; i--) {
		if (pmo_cap[i] != 0)
			cap_free(process, pmo_cap[i]);
	}
 out_fail:
	return r;
}

/* defined in page_table.S */
extern void flush_idcache(void);

extern void prepare_env(char *env, u64 top_vaddr,
			struct process_metadata *meta, char *name);

/*
 * main_thread: the first thread in a process (process).
 * So, thread_create_main needs to load the code/data as well.
 */
int thread_create_main(struct process *process, u64 stack_base,
		       u64 stack_size, u32 prio, u32 type, s32 aff,
		       const char *bin_start, char *bin_name)
{
	int ret, thread_cap, stack_pmo_cap;
	struct thread *thread;
	struct pmobject *stack_pmo;
	struct vmspace *init_vmspace;
	struct process_metadata meta;
	u64 stack;
	u64 pc;

	init_vmspace = obj_get(process, VMSPACE_OBJ_ID, TYPE_VMSPACE);
	obj_put(init_vmspace);

	/* Allocate and setup a user stack for the init thread */
	stack_pmo = obj_alloc(TYPE_PMO, sizeof(*stack_pmo));
	if (!stack_pmo) {
		ret = -ENOMEM;
		goto out_fail;
	}
	pmo_init(stack_pmo, PMO_DATA, stack_size, 0);
	stack_pmo_cap = cap_alloc(process, stack_pmo, 0);
	if (stack_pmo_cap < 0) {
		ret = stack_pmo_cap;
		goto out_free_obj_pmo;
	}

	ret = vmspace_map_range(init_vmspace, stack_base, stack_size,
				VMR_READ | VMR_WRITE, stack_pmo);
	BUG_ON(ret != 0);

	/* init thread */
	thread = obj_alloc(TYPE_THREAD, sizeof(*thread));
	if (!thread) {
		ret = -ENOMEM;
		goto out_free_cap_pmo;
	}

	/* Fill the parameter of the thread struct */
	stack = stack_base + stack_size;

	pc = load_binary(process, init_vmspace, bin_start, &meta);

	prepare_env((char *)phys_to_virt(stack_pmo->start) + stack_size,
		    stack, &meta, bin_name);
	stack -= ENV_SIZE_ON_STACK;

	ret = thread_init(thread, process, stack, pc, prio, type, aff);
	BUG_ON(ret != 0);

	thread_cap = cap_alloc(process, thread, 0);
	if (thread_cap < 0) {
		ret = thread_cap;
		goto out_free_obj_thread;
	}

	/* L1 icache & dcache have no coherence */
	flush_idcache();

	// return thread;
	return thread_cap;
 out_free_obj_thread:
	obj_free(thread);
 out_free_cap_pmo:
	cap_free(process, stack_pmo_cap);
	stack_pmo = NULL;
 out_free_obj_pmo:
	obj_free(stack_pmo);
 out_fail:
	return ret;
}

/*
 * exported functions
 */
void switch_thread_vmspace_to(struct thread *thread)
{
	switch_vmspace_to(thread->vmspace);
}

/*
 * Syscalls
 */

/* Exit the current running thread */
void sys_exit(int ret)
{
	int cpuid = 0;
	struct thread *target = current_threads[cpuid];

	kinfo("sys_exit with value %d\n", ret);
	/* Set thread state */
	obj_free(target);

	/* Set current running thread to NULL */
	current_threads[cpuid] = NULL;
	break_point();
}
