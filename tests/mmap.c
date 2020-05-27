#include "common.h"
#define SIZE 0x3000

int main() {
	int fd = do_open0();
	int bfd = do_create_buf(fd, SIZE);
	char *buffer = (char *) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);

	if (buffer == MAP_FAILED)
		syserr("mmap");

	for (int i = 0; i < SIZE; i++)
		buffer[i] = 0x13;

	do_munmap(buffer, SIZE);
	do_close(bfd);
	do_close(fd);
	return 0;
}
