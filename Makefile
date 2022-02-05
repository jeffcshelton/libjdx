CC = clang
CFLAGS = -std=c17 -Iinclude -Ilibdeflate -Wall -pedantic

RELEASE_FLAGS = -DRELEASE -fomit-frame-pointer -O3
DEBUG_FLAGS = -DDEBUG -g -fno-omit-frame-pointer -O0

SRCS := $(wildcard src/*.c src/**/*.c)
RELEASE_OBJS := $(patsubst src/%.c,build/release/%_c.o,$(SRCS))
DEBUG_OBJS := $(patsubst src/%.c,build/debug/%_c.o,$(SRCS))

LIBDEFLATE_OBJS = build/libdeflate/*.o

TEST_SRCS := $(wildcard tests/*.c)
TEST_OBJS := $(patsubst tests/%.c,build/tests/%_c.o,$(TEST_SRCS))

_ = $(shell git submodule update --init --recursive)

.PHONY: libjdx install uninstall tests clean

libjdx: lib/libjdx.a
debug: lib/libjdx_debug.a

lib/libjdx.a: $(RELEASE_OBJS) $(LIBDEFLATE_OBJS)
	@mkdir -p lib
	@ar -r lib/libjdx.a $^

lib/libjdx_debug.a: $(DEBUG_OBJS) $(LIBDEFLATE_OBJS)
	@mkdir -p lib
	@ar cr lib/libjdx_debug.a $^

install: lib/libjdx.a
	cp -r include/libjdx.h /usr/local/include
	cp -r lib/libjdx.a /usr/local/lib

uninstall:
	rm -f /usr/local/include/libjdx.h
	rm -f /usr/local/lib/libjdx.a

tests: lib/libjdx.a $(TEST_OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $^ -o bin/tests

build/release/%_c.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -c $^ -o $@

build/debug/%_c.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $^ -o $@

build/tests/%_c.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $^ -o $@

build/libdeflate/*.o: libdeflate/libdeflate.a
	@mkdir -p $(dir $@)
	cd build/libdeflate && ar x ../../$<

libdeflate/libdeflate.a:
	cd libdeflate && make libdeflate.a

clean:
	@cd libdeflate && make clean
	@rm -rf build lib bin