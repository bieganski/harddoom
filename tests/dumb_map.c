#include "common.h"
#define SIZE 0x3000

int main() {
	int fd = do_open0();
	int bfd = do_create_buf(fd, SIZE);
	do_map_buf_with_err(fd, 1);
	do_close(bfd);
	do_close(fd);
	return 0;
}
