#include "common.h"

int main() {
	int fd = do_open0();
	do_wait(fd, 0xfffffff);
	do_close(fd);
	return 0;
}

