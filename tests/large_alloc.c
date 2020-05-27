#include "common.h"
#define SIZE 4096
#define COUNT 2048

int addrs[COUNT];
char *bufs[COUNT];

static void check_line(int id, int last) {
	for (int i = 0; i < 0x40; i++)
		for (int j = 0; j < 0x40; j++) {
		const char expected = (i == j && i <= last) ? 0xaa : 0x00;
			if (bufs[id][i * 0x40 + j] != expected) {
				fprintf(stderr, "Invalid value at (%d, %d)\n", i, j);
				exit(-1);
			}
		}
}

int main() {
	int fd = do_open0();

	for (int i = 0; i < COUNT; i++) {
		int bfd = do_create_buf(fd, SIZE);
		addrs[i] = do_map_buf(fd, bfd, 0);
	       	bufs[i] = (char *) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
		if (bufs[i] == MAP_FAILED)
			syserr("mmap");
		do_close(bfd);
	}


	uint32_t *cmd = (uint32_t*) bufs[0];
	cmd[0] = UHARDDOOM_USER_DRAW_LINE_HEADER(0xaa);
	cmd[2] = 0x40;
	cmd[3] = UHARDDOOM_USER_DRAW_LINE_W3(0, 0);

	for (int i = 1; i < COUNT; i++) {
		const int last = 1 + i % 0x20;
		cmd[4] = UHARDDOOM_USER_DRAW_LINE_W4(last, last);
		cmd[1] = addrs[i];
		do_run(fd, addrs[0], sizeof(uint32_t) * 5);
		do_wait(fd, 0);
		check_line(i, last);
	}

	for (int i = 0; i < COUNT; i++) {
		do_unmap_buf(fd, addrs[i]);
		do_munmap(bufs[i], SIZE);
	}

	do_close(fd);
	return 0;
}
