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

#include <common/mm.h>
#include <common/kprint.h>
#include <common/macro.h>

#include "buddy.h"
#include "slab.h"

extern unsigned long *img_end;

#define PHYSICAL_MEM_START (24*1024*1024)	//24M

#define START_VADDR phys_to_virt(PHYSICAL_MEM_START)	//24M
#define NPAGES (128*1000)

#define PHYSICAL_MEM_END (PHYSICAL_MEM_START+NPAGES*BUDDY_PAGE_SIZE)

/*
 * Layout:
 *
 * | metadata (npages * sizeof(struct page)) | start_vaddr ... (npages * PAGE_SIZE) |
 *
 */

unsigned long get_ttbr1(void)
{
	unsigned long pgd;

	__asm__("mrs %0,ttbr1_el1":"=r"(pgd));
	return pgd;
}

/*
 * map_kernel_space: map the kernel virtual address
 * [va:va+size] to physical addres [pa:pa+size].
 * 1. get the kernel pgd address
 * 2. fill the block entry with corresponding attribution bit
 *
 */
void map_kernel_space(vaddr_t va, paddr_t pa, size_t len)
{
	// <lab2>
	#define IS_VALID (1UL << 0)
	#define UXN	       (0x1UL << 54)
	#define ACCESSED       (0x1UL << 10)
	#define INNER_SHARABLE (0x3UL << 8)
	#define NORMAL_MEMORY  (0x4UL << 2)

	#define GET_L0_INDEX(x) (((x) >> (12 + 9 + 9 + 9)) & 0x1ff)
	#define GET_L1_INDEX(x) (((x) >> (12 + 9 + 9)) & 0x1ff)
	#define GET_L2_INDEX(x) (((x) >> (12 + 9)) & 0x1ff)
	u64 *pgd = get_ttbr1() + KBASE;
	len = ROUND_UP(len, (PAGE_SIZE << 9));
	for (size_t mapped = 0; mapped < len; mapped += (PAGE_SIZE << 9)) {
		u32 l0_idx = GET_L0_INDEX(va + mapped);
		u64 *l1_tbl = (pgd[l0_idx] & ~0xFF) + KBASE;
		u32 l1_idx = GET_L1_INDEX(va + mapped);
		u64 *l2_tbl = (l1_tbl[l1_idx] & ~0xFF) + KBASE;
		u32 l2_idx = GET_L2_INDEX(va + mapped);
		l2_tbl[l2_idx] = (pa + mapped) | UXN	/* Unprivileged execute never */
		    | ACCESSED	/* Set access flag */
		    | INNER_SHARABLE	/* Sharebility */
		    | NORMAL_MEMORY	/* Normal memory */
		    | IS_VALID;
	}
	// </lab2>
}

void kernel_space_check(void)
{
	unsigned long kernel_val;
	for (unsigned long i = 128; i < 256; i++) {
		kernel_val = *(unsigned long *)(KBASE + (i << 21));
		kinfo("kernel_val: %lx\n", kernel_val);
	}
	kinfo("kernel space check pass\n");
}

struct phys_mem_pool global_mem;

void mm_init(void)
{
	vaddr_t free_mem_start = 0;
	struct page *page_meta_start = NULL;
	u64 npages = 0;
	u64 start_vaddr = 0;

	free_mem_start =
	    phys_to_virt(ROUND_UP((vaddr_t) (&img_end), PAGE_SIZE));
	npages = NPAGES;
	start_vaddr = START_VADDR;
	kdebug("[CHCORE] mm: free_mem_start is 0x%lx, free_mem_end is 0x%lx\n",
	       free_mem_start, phys_to_virt(PHYSICAL_MEM_END));

	if ((free_mem_start + npages * sizeof(struct page)) > start_vaddr) {
		BUG("kernel panic: init_mm metadata is too large!\n");
	}

	page_meta_start = (struct page *)free_mem_start;
	kdebug("page_meta_start: 0x%lx, real_start_vadd: 0x%lx,"
	       "npages: 0x%lx, meta_page_size: 0x%lx\n",
	       page_meta_start, start_vaddr, npages, sizeof(struct page));

	/* buddy alloctor for managing physical memory */
	init_buddy(&global_mem, page_meta_start, start_vaddr, npages);

	/* slab alloctor for allocating small memory regions */
	init_slab();

	map_kernel_space(KBASE + (128UL << 21), 128UL << 21, 128UL << 21);
	//check whether kernel space [KABSE + 256 : KBASE + 512] is mapped 
	kernel_space_check();
}
