#include "common.h"


static void chk(int fd, int req, char *err) {
	if (ioctl(fd, req, 0x01) != -1) {
		fprintf(stderr, "ioctl %s succeeded\n", err);
		exit(-1);
	}
}

int main() {
	int fd = do_open0();
	chk(fd, UDOOMDEV_IOCTL_MAP_BUFFER, "map_buffer");
	chk(fd, UDOOMDEV_IOCTL_CREATE_BUFFER, "create_buffer");
	chk(fd, UDOOMDEV_IOCTL_RUN, "run");
	chk(fd, UDOOMDEV_IOCTL_WAIT, "wait");
	
	do_close(fd);
	return 0;
}
