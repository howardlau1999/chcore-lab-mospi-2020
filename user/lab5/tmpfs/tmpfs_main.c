#include <syscall.h>
#include <launcher.h>
#include "tmpfs_server.h"

#define server_ready_flag_offset 0x0
#define server_exit_flag_offset  0x4

static void fs_dispatch(ipc_msg_t * ipc_msg)
{
	int ret = 0;

	if (ipc_msg->data_len >= 4) {
		struct fs_request *fr = (struct fs_request *)
		    ipc_get_msg_data(ipc_msg);
		switch (fr->req) {
		case FS_REQ_SCAN:{
				if (ipc_msg->cap_slot_number != 1) {
					ret = -EINVAL;
					break;
				}
				int cap = ipc_get_msg_cap(ipc_msg, 0);
				ret = usys_map_pmo(SELF_CAP, cap, fr->buff, VM_READ | VM_WRITE);
				if (ret < 0) {
					break;
				}
				ret = fs_server_scan(fr->path, fr->offset, fr->buff, fr->count);
				usys_unmap_pmo(SELF_CAP, cap, fr->buff);
				break;
			}
		case FS_REQ_MKDIR:
			ret = fs_server_mkdir(fr->path);
			break;
		case FS_REQ_RMDIR:
			ret = fs_server_rmdir(fr->path);
			break;
		case FS_REQ_CREAT:
			ret = fs_server_creat(fr->path);
			break;
		case FS_REQ_UNLINK:
			ret = fs_server_unlink(fr->path);
			break;
		case FS_REQ_OPEN:
			error("%s: %d Not impelemented yet\n", __func__,
			      ((int *)ipc_get_msg_data(ipc_msg))[0]);
			usys_exit(-1);
			break;
		case FS_REQ_CLOSE:
			error("%s: %d Not impelemented yet\n", __func__,
			      ((int *)ipc_get_msg_data(ipc_msg))[0]);
			usys_exit(-1);
			break;
		case FS_REQ_WRITE:{
				if (ipc_msg->cap_slot_number != 1) {
					ret = -EINVAL;
					break;
				}
				int cap = ipc_get_msg_cap(ipc_msg, 0);
				ret = usys_map_pmo(SELF_CAP, cap, fr->buff, VM_READ | VM_WRITE);
				if (ret < 0) {
					break;
				}
				ret = fs_server_write(fr->path, fr->offset, fr->buff, fr->count);
				usys_unmap_pmo(SELF_CAP, cap, fr->buff);
				break;
			}
		case FS_REQ_READ:{
				if (ipc_msg->cap_slot_number != 1) {
					ret = -EINVAL;
					break;
				}
				int cap = ipc_get_msg_cap(ipc_msg, 0);
				ret = usys_map_pmo(SELF_CAP, cap, fr->buff, VM_READ | VM_WRITE);
				if (ret < 0) {
					break;
				}
				ret = fs_server_read(fr->path, fr->offset, fr->buff, fr->count);
				usys_unmap_pmo(SELF_CAP, cap, fr->buff);
				break;
			}
		case FS_REQ_GET_SIZE:{
				ret = fs_server_get_size(fr->path);
				break;

			}
		default:
			error("%s: %d Not impelemented yet\n", __func__,
			      ((int *)ipc_get_msg_data(ipc_msg))[0]);
			usys_exit(-1);
			break;
		}
	} else {
		printf("TMPFS: no operation num\n");
		usys_exit(-1);
	}
	usys_ipc_return(ret);
}

int main(int argc, char *argv[], char *envp[])
{
	void *info_page_addr = (void *)(long)TMPFS_INFO_VADDR;
	// void *info_page_addr = (void *) (envp[0]);
	int *server_ready_flag;
	int *server_exit_flag;

	printf("info_page_addr: 0x%lx\n", info_page_addr);

	if (info_page_addr == NULL) {
		error("[tmpfs] no info received. Bye!\n");
		usys_exit(-1);
	}

	fs_server_init(CPIO_BIN);
	info("register server value = %u\n", ipc_register_server(fs_dispatch));

	server_ready_flag = info_page_addr + server_ready_flag_offset;
	*server_ready_flag = 1;

	server_exit_flag = info_page_addr + server_exit_flag_offset;
	while (*server_exit_flag != 1) {
		usys_yield();
	}

	info("exit now. Bye!\n");
	return 0;
}
