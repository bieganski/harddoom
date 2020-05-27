#include "common.h"
#define SIZE 0x3000

int main() {
	int fd = do_open0();
	int bfd = do_create_buf(fd, SIZE);
	int addr = do_map_buf(fd, bfd, 0);

	char *buffer = (char *) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	if (buffer == MAP_FAILED)
		syserr("mmap");
	do_close(bfd);

	uint32_t *cmd = (uint32_t*) (buffer + 0x1000);
	cmd[0] = UHARDDOOM_USER_DRAW_LINE_HEADER(0xaa);
	cmd[1] = addr + SIZE;
	cmd[2] = 0x40;
	cmd[3] = UHARDDOOM_USER_DRAW_LINE_W3(0, 0);
	cmd[4] = UHARDDOOM_USER_DRAW_LINE_W4(31, 31);
	do_run_and_wait_with_err(fd, addr + 0x1000, sizeof(uint32_t) * 5);

	do_unmap_buf(fd, addr);
	do_munmap(buffer, SIZE);
	do_close(fd);
	return 0;
}
