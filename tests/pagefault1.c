#include "common.h"

int main() {
	int fd = do_open0();

	do_run_and_wait_with_err(fd, 0x1000, 0x10);
	do_close(fd);
	return 0;
}
