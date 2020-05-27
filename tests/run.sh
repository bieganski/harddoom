#!/bin/sh
TESTS="buf_zero crossdraw crossmap draw draw_double_mmap dumb_map invalid_cmd2 invalid_cmd invalid_ioctl large_wait mmap pagefault1 partial_op pagefault2 large_alloc overflow unmap_invalid_addr large_op draw_ro"


for x in $TESTS; do
	echo === $x ===
	if ./$x ; then
		echo OK
	else
		echo Failed
	fi
done

