#include "common.h"
#define DIM 1024
#define CMDS 1024
#define CMD_SIZE 0x8000

int main() {
	int fd = do_open0();
	int bfd = do_create_buf(fd, 4 * DIM * DIM);
	int addr = do_map_buf(fd, bfd, 0);

	int cmdfd = do_create_buf(fd, CMD_SIZE);
	int cmd_addr = do_map_buf(fd, cmdfd, 0);

	uint32_t *cmd = (uint32_t *) mmap(0, CMD_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, cmdfd, 0);
	if (cmd == MAP_FAILED)
		syserr("mmap");

	do_close(bfd);
	do_close(cmdfd);

	for (int i = 0; i < CMDS; i++) {
		cmd[5 * i + 0] = UHARDDOOM_USER_FILL_RECT_HEADER(0xaa);
		cmd[5 * i + 1] = addr;
		cmd[5 * i + 2] = 2 * DIM;
		cmd[5 * i + 3] = UHARDDOOM_USER_FILL_RECT_W3(0, 0);
		cmd[5 * i + 4] = UHARDDOOM_USER_FILL_RECT_W4(DIM, DIM);
	}
	do_run(fd, cmd_addr, sizeof(uint32_t) * 5 * CMDS);
	
	do_unmap_buf(fd, addr);
	do_unmap_buf(fd, cmd_addr);
	do_munmap(cmd, CMD_SIZE);
	do_close(fd);
	return 0;
}
