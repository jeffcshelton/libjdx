CC = gcc
CFLAGS = -std=c17 -Iinclude -I/usr/local/include -Wall -pedantic -O3
LD_FLAGS = -L/usr/local/lib

UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
	DYN_EXT = dylib

	ifneq ($(wildcard /usr/local/lib/libdeflate.dylib),)
		CFLAGS += -DLIBDEFLATE_AVAILABLE
		LD_FLAGS += -ldeflate
	endif
else
	DYN_EXT = so

	ifneq ($(wildcard /usr/local/lib/libdeflate.so),)
		CFLAGS += -DLIBDEFLATE_AVAILABLE
		LD_FLAGS += -ldeflate
	endif
endif

SRCS := $(wildcard src/*.c src/**/*.c)
OBJS := $(patsubst %.c,%_c.o,$(subst src/,build/,$(SRCS)))

TEST_SRCS := $(wildcard tests/*.c)
TEST_OBJS := $(patsubst %.c,%_c.o,$(subst tests/,build/tests/,$(TEST_SRCS)))

.PHONY: jdx install uninstall tests clean

jdx: jdx-static jdx-dynamic

install: jdx
	rm -rf /usr/local/include/jdx
	cp -r include/jdx /usr/local/include
	cp -r lib/libjdx.a lib/libjdx.$(DYN_EXT) /usr/local/lib

uninstall:
	rm -rf /usr/local/include/jdx
	rm -f /usr/local/lib/libjdx.a /usr/local/lib/libjdx.$(DYN_EXT)

tests: $(OBJS) $(TEST_OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LD_FLAGS) $^ -o bin/tests

jdx-static: $(OBJS)
	@mkdir -p lib
	@ar -r lib/libjdx.a $^

jdx-dynamic: $(OBJS)
	@mkdir -p lib
	$(CC) $(LD_FLAGS) -shared -fpic $^ -o lib/libjdx.$(DYN_EXT)

build/%_c.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $^ -o $@

build/tests/%_c.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@rm -rf build lib bin