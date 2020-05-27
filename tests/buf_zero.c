#include "common.h"

int main() {
	int fd = do_open0();
	do_create_buf_with_err(fd, 0);
	do_close(fd);
	return 0;
}
