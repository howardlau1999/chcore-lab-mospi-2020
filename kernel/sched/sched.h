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

#pragma once

#include <common/registers.h>
#include <common/types.h>
#include <common/list.h>
#include <common/kprint.h>

#include <common/machine.h>

struct thread;

/* BUDGET represents the number of TICKs */
#define DEFAULT_BUDGET	2
#define TICK_MS		500

#define MAX_PRIO	255
#define MIN_PRIO	0
#define PRIO_NUM	(MAX_PRIO + 1)

#define NO_AFF		-1

/* Data structures */

#define	STATE_STR_LEN	20
enum thread_state {
	TS_INIT = 0,
	TS_READY,
	/* intermediate stat used by sched */
	TS_INTER,
	TS_RUNNING,
	TS_EXIT,
	/* waiting IPC or etc */
	TS_WAITING,
	/* waiting to be exited */
	TS_EXITING,
};

#define TYPE_STR_LEN	20
enum thread_type {
	TYPE_IDLE = 0,
	/* ROOT thread has all cap, it is also a user thread */
	TYPE_ROOT,
	TYPE_USER,
	TYPE_SHADOW,
	TYPE_KERNEL,
	TYPE_TESTS
};

typedef struct sched_cont {
	u32 budget;
	char pad[pad_to_cache_line(sizeof(u32))];
} sched_cont_t;

/* size in registers.h (to be used in asm) */
typedef struct arch_exec_cont {
	u64 reg[REG_NUM];
} arch_exec_cont_t;

struct thread_ctx {
	/* Executing Context */
	arch_exec_cont_t ec;

	/* Thread Type */
	u32 type;
};

extern char thread_type[][TYPE_STR_LEN];
extern char thread_state[][STATE_STR_LEN];

u64 switch_context(void);

