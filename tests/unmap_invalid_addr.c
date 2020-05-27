#include "common.h"
#define SIZE 0x3000

int main() {
	int fd = do_open0();
	do_unmap_buf_with_err(fd, 0x1000);
	do_close(fd);
	return 0;
}
