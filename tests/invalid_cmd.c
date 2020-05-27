#include "common.h"
#define SIZE 0x1000

int main() {
	int fd = do_open0();
	int bfd = do_create_buf(fd, SIZE);
	int addr = do_map_buf(fd, bfd, 0);

	char *buffer = (char *) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	if (buffer == MAP_FAILED)
		syserr("mmap");

	for (int i = 0; i < 0x10; i++)
		buffer[i] = i % 2 ? 0x13 : 0x31;

	do_run_and_wait_with_err(fd, addr, 0x10);

	do_unmap_buf(fd, addr);
	do_close(bfd);
	do_close(fd);
	return 0;
}
