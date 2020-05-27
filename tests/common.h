#ifndef COMMON_H
#define COMMON_H
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

void syserr(const char *fmt);
void err(char *msg);
void do_run(int fd, uint32_t addr, uint32_t size);
void do_run_with_err(int fd, uint32_t addr, uint32_t size);
void do_wait(int fd, uint32_t cnt);
void do_wait_for_err(int fd, uint32_t cnt);
void do_run_and_wait_with_err(int fd, uint32_t addr, uint32_t size);
int do_open(char *path);
void do_close(int fd);
void do_munmap(void *addr, size_t len);
int do_open0();
int do_open1();
int do_create_buf(int fd, int size);
void do_create_buf_with_err(int fd, int size);
int do_map_buf(int fd, int bfd, int ro);
void do_map_buf_with_err(int fd, int bfd);
void do_unmap_buf(int fd, uint32_t addr);
void do_unmap_buf_with_err(int fd, uint32_t addr);
#endif
