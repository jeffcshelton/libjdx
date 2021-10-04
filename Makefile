CC = gcc
CFLAGS = -std=c17 -Iinclude -I/usr/local/include -Wall -pedantic -O3

SRCS := $(wildcard src/*.c src/**/*.c)
OBJS := $(patsubst %.c,%_c.o,$(subst src/,build/,$(SRCS))) libdeflate/libdeflate.a

TEST_SRCS := $(wildcard tests/*.c)
TEST_OBJS := $(patsubst %.c,%_c.o,$(subst tests/,build/tests/,$(TEST_SRCS)))

.PHONY: install uninstall tests clean

libjdx.a: $(OBJS)
	@mkdir -p lib
	@ar -r lib/libjdx.a $^

install: libjdx.a
	rm -rf /usr/local/include/jdx
	cp -r include/jdx /usr/local/include
	cp -r lib/libjdx.a /usr/local/lib

uninstall:
	rm -rf /usr/local/include/jdx
	rm -f /usr/local/lib/libjdx.a

tests: $(OBJS) $(TEST_OBJS)
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

clean:
	@rm -rf build lib bin