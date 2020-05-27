#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include "udoomdev.h"
#include "uharddoom.h"

void syserr(const char *fmt) {
	fprintf(stderr,"ERROR %s (%d; %s)\n", fmt, errno, strerror(errno));
	exit(1);
}

void err(char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

void do_run(int fd, uint32_t addr, uint32_t size) {
	struct udoomdev_ioctl_run run = {addr, size};
	if (ioctl(fd, UDOOMDEV_IOCTL_RUN, &run))
		syserr("run");
}

void do_run_with_err(int fd, uint32_t addr, uint32_t size) {
	struct udoomdev_ioctl_run run = {addr, size};
	if (ioctl(fd, UDOOMDEV_IOCTL_RUN, &run) != -1)
		err("run should fail");
}

void do_wait(int fd, uint32_t cnt) {
	struct udoomdev_ioctl_wait wait = {cnt};
	if (ioctl(fd, UDOOMDEV_IOCTL_WAIT, &wait))
		syserr("wait");
}

void do_wait_for_err(int fd, uint32_t cnt) {
	struct udoomdev_ioctl_wait wait = {cnt};
	if (ioctl(fd, UDOOMDEV_IOCTL_WAIT, &wait) != -1)
		err("wait should fail");
}


void do_run_and_wait_with_err(int fd, uint32_t addr, uint32_t size) {
	struct udoomdev_ioctl_run run = {addr, size};
	ioctl(fd, UDOOMDEV_IOCTL_RUN, &run); // ignore
	do_wait_for_err(fd, 0);
}

int do_open(char *path) {
	int fd;
	if ((fd = open(path, O_RDWR)) < 0)
		syserr("open");
}

void do_close(int fd) {
	if (close(fd) < 0)
		syserr("close");
}

void do_munmap(void *addr, size_t len) {
	if (munmap(addr, len) < 0)
		syserr("munmap");
}

int do_open0() {
	return do_open("/dev/udoom0");
}

int do_open1() {
	return do_open("/dev/udoom1");
}

int do_create_buf(int fd, int size) {
	struct udoomdev_ioctl_create_buffer cb = { size };
	int bfd;
	if ((bfd = ioctl(fd, UDOOMDEV_IOCTL_CREATE_BUFFER, &cb)) < 0)
		syserr("create_buffer");
	return bfd;
}

void do_create_buf_with_err(int fd, int size) {
	struct udoomdev_ioctl_create_buffer cb = { size };
	if (ioctl(fd, UDOOMDEV_IOCTL_CREATE_BUFFER, &cb) != -1)
		err("create buffer should fail");
}

int do_map_buf(int fd, int bfd, int ro) {
	struct udoomdev_ioctl_map_buffer mbf = {bfd, ro};
	int addr = ioctl(fd, UDOOMDEV_IOCTL_MAP_BUFFER, &mbf);
	if (addr == -1)
		syserr("map_buffer");
	return addr;
}

void do_map_buf_with_err(int fd, int bfd) {
	struct udoomdev_ioctl_map_buffer mbf = {bfd, 0};
	if (ioctl(fd, UDOOMDEV_IOCTL_MAP_BUFFER, &mbf) != -1)
		err("map_buf should fail");
}

void do_unmap_buf(int fd, uint32_t addr) {
	struct udoomdev_ioctl_unmap_buffer umbf = { addr };
	if (ioctl(fd, UDOOMDEV_IOCTL_UNMAP_BUFFER, &umbf) < 0)
		syserr("unmap buffer");
}

void do_unmap_buf_with_err(int fd, uint32_t addr) {
	struct udoomdev_ioctl_unmap_buffer umbf = { addr };
	if (ioctl(fd, UDOOMDEV_IOCTL_UNMAP_BUFFER, &umbf) != -1)
		err("unmap buffer should fail");
}
