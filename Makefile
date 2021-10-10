CC = gcc
CFLAGS = -std=c17 -Iinclude -Ilibdeflate -Wall -pedantic -O3

SRCS := $(wildcard src/*.c src/**/*.c)
LIBJDX_OBJS := $(patsubst %.c,%_c.o,$(subst src/,build/,$(SRCS)))
LIBDEFLATE_OBJS = libdeflate/lib/*.o libdeflate/lib/x86/*.o

TEST_SRCS := $(wildcard tests/*.c)
TEST_OBJS := $(patsubst %.c,%_c.o,$(subst tests/,build/tests/,$(TEST_SRCS)))

.PHONY: libjdx install uninstall tests clean
libjdx: lib/libjdx.a

lib/libjdx.a: $(LIBJDX_OBJS) $(LIBDEFLATE_OBJS)
	@mkdir -p lib
	@ar -r lib/libjdx.a $^

install: lib/libjdx.a
	cp -r include/libjdx.h /usr/local/include
	cp -r lib/libjdx.a /usr/local/lib

uninstall:
	rm -f /usr/local/include/libjdx.h
	rm -f /usr/local/lib/libjdx.a

tests: lib/libjdx.a $(TEST_OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $^ -o bin/tests

build/%_c.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $^ -o $@

build/tests/%_c.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $^ -o $@

libdeflate/libdeflate.a:
	cd libdeflate && make libdeflate.a

libdeflate/lib/*.o: libdeflate/libdeflate.a
libdeflate/lib/x86/*.o: libdeflate/libdeflate.a

clean:
	@cd libdeflate && make clean
	@rm -rf build lib bin