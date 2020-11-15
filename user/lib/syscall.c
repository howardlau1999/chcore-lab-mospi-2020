#include <lib/print.h>
#include <lib/syscall.h>
#include <lib/type.h>

u64 syscall(u64 sys_no, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4,
	    u64 arg5, u64 arg6, u64 arg7, u64 arg8)
{

	u64 ret = 0;
	/*
	 * Lab3: Your code here
	 * Use inline assembly to store arguments into x0 to x7, store syscall number to x8,
	 * And finally use svc to execute the system call. After syscall returned, don't forget
	 * to move return value from x0 to the ret variable of this function
	 */
	asm ("mov x8, %1\n"
		"mov x0, %2\n"
		"mov x1, %3\n"
		"mov x2, %4\n"
		"mov x3, %5\n"
		"mov x4, %6\n"
		"mov x5, %7\n"
		"mov x6, %8\n"
		"mov x7, %9\n"
		"svc #0\n"
		"mov %0, x0\n"
	: "=r"(ret) 
	: "r"(sys_no), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5), "r"(arg6), "r"(arg7), "r"(arg8)
	);

	return ret;
}

/*
 * Lab3: your code here:
 * Finish the following system calls using helper function syscall
 */
void usys_putc(char ch)
{
	syscall(SYS_putc, ch, 0, 0, 0, 0, 0, 0, 0, 0);
}

void usys_exit(int ret)
{
	syscall(SYS_exit, ret, 0, 0, 0, 0, 0, 0, 0, 0);
}

int usys_create_pmo(u64 size, u64 type)
{
	return syscall(SYS_create_pmo, size, type, 0, 0, 0, 0, 0, 0, 0);
}

int usys_map_pmo(u64 process_cap, u64 pmo_cap, u64 addr, u64 rights)
{
	return syscall(SYS_map_pmo, process_cap, pmo_cap, addr, rights, 0, 0, 0, 0, 0);
}

u64 usys_handle_brk(u64 addr)
{
	return syscall(SYS_handle_brk, addr, 0, 0, 0, 0, 0, 0, 0, 0);
}

/* Here finishes all syscalls need by lab3 */

u32 usys_getc(void)
{
	return (u32) syscall(SYS_getc, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

u64 usys_yield(void)
{
	return syscall(SYS_yield, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int usys_create_device_pmo(u64 paddr, u64 size)
{
	return syscall(SYS_create_device_pmo, paddr, size, 0, 0, 0, 0, 0, 0, 0);
}

int usys_unmap_pmo(u64 process_cap, u64 pmo_cap, u64 addr)
{
	return syscall(SYS_unmap_pmo, process_cap, pmo_cap, addr,
		       0, 0, 0, 0, 0, 0);
}

int usys_set_affinity(u64 thread_cap, s32 aff)
{
	return syscall(SYS_set_affinity, thread_cap, (u64) aff, 0, 0, 0, 0, 0,
		       0, 0);
}

s32 usys_get_affinity(u64 thread_cap)
{
	return syscall(SYS_get_affinity, thread_cap, 0, 0, 0, 0, 0, 0, 0, 0);
}

/*
 * Lab4
 * Add syscall
 */

u32 usys_get_cpu_id(void)
{
	return (u32) syscall(SYS_get_cpu_id, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int usys_create_thread(u64 process_cap, u64 stack, u64 pc, u64 arg, u32 prio,
		       s32 aff)
{
	return syscall(SYS_create_thread, process_cap, stack, pc, arg,
		       (u64) prio, (u64) aff, 0, 0, 0);
}

int usys_create_process(void)
{
	return syscall(SYS_create_process, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

u64 usys_register_server(u64 callback, u64 max_client, u64 vm_config_ptr)
{
	return syscall(SYS_register_server, callback, max_client, vm_config_ptr,
		       0, 0, 0, 0, 0, 0);
}

u32 usys_register_client(u32 server_cap, u64 vm_config_ptr)
{
	return syscall(SYS_register_client, server_cap, vm_config_ptr,
		       0, 0, 0, 0, 0, 0, 0);
}

u64 usys_ipc_call(u32 conn_cap, u64 arg0)
{
	return syscall(SYS_ipc_call, conn_cap, arg0, 0, 0, 0, 0, 0, 0, 0);
}

u64 usys_ipc_reg_call(u32 conn_cap, u64 arg0)
{
	return syscall(SYS_ipc_reg_call, conn_cap, arg0, 0, 0, 0, 0, 0, 0, 0);
}

void usys_ipc_return(u64 ret)
{
	syscall(SYS_ipc_return, ret, 0, 0, 0, 0, 0, 0, 0, 0);
}

int usys_debug(void)
{
	return syscall(SYS_debug, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int usys_cap_copy_to(u64 dest_process_cap, u64 src_slot_id)
{
	return syscall(SYS_cap_copy_to, dest_process_cap, src_slot_id,
		       0, 0, 0, 0, 0, 0, 0);
}

int usys_cap_copy_from(u64 src_process_cap, u64 src_slot_id)
{
	return syscall(SYS_cap_copy_from, src_process_cap, src_slot_id,
		       0, 0, 0, 0, 0, 0, 0);
}

int usys_fs_load_cpio(u64 vaddr)
{
	return syscall(SYS_fs_load_cpio, vaddr, 0, 0, 0, 0, 0, 0, 0, 0);
}

int usys_create_pmos(void *req, u64 cnt)
{
	return syscall(SYS_create_pmos, (u64) req, cnt, 0, 0, 0, 0, 0, 0, 0);
}

int usys_map_pmos(u64 cap, void *req, u64 cnt)
{
	return syscall(SYS_map_pmos, cap, (u64) req, cnt, 0, 0, 0, 0, 0, 0);
}

int usys_write_pmo(u64 cap, u64 offset, void *buf, u64 size)
{
	return syscall(SYS_write_pmo, cap, offset, (u64) buf, size,
		       0, 0, 0, 0, 0);
}

int usys_read_pmo(u64 cap, u64 offset, void *buf, u64 size)
{
	return syscall(SYS_read_pmo, cap, offset, (u64) buf, size,
		       0, 0, 0, 0, 0);
}

int usys_transfer_caps(u64 process, int *src_caps, int nr, int *dst_caps)
{
	return syscall(SYS_transfer_caps, process, (u64) src_caps,
		       (u64) nr, (u64) dst_caps, 0, 0, 0, 0, 0);
}

void usys_top(void)
{
	syscall(SYS_top, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
