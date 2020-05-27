TESTS=buf_zero crossdraw crossmap draw draw_double_mmap dumb_map invalid_cmd2 invalid_cmd invalid_ioctl large_wait \
      mmap pagefault1 partial_op pagefault2 draw_ro large_alloc overflow unmap_invalid_addr large_op

all: $(TESTS)

$(TESTS): %: %.o common.o
	$(CC) -o $@ $< common.o

clean:
	rm -f $(TESTS) *.o
