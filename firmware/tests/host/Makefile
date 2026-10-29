CC ?= gcc
CFLAGS = -Wall -Wextra -std=c11 -I../../components/scope_proto/include

test: test_proto
	./test_proto

test_proto: test_proto.c ../../components/scope_proto/scope_proto.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f test_proto

.PHONY: test clean
