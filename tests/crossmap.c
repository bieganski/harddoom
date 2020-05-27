#include "common.h"
#define SIZE 0x3000

int main() {
	int fd = do_open0();
	int fd2 = do_open1();
	int bfd = do_create_buf(fd, SIZE);
	do_map_buf_with_err(fd2, bfd);

	do_close(bfd);
	do_close(fd);
	do_close(fd2);

	return 0;
}
