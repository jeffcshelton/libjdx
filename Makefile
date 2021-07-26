CC = gcc
CFLAGS = -std=c17 -I./include -Wall -O3

UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
	DYN_EXT = dylib
else
	DYN_EXT = so
endif

SRCS := $(wildcard src/*.c src/**/*.c)
OBJS := $(patsubst %.c,%_c.o,$(subst src/,build/,$(SRCS)))

.PHONY: jdx install uninstall clean

jdx: jdx-static jdx-dynamic

install: jdx
	rm -rf /usr/local/include/jdx
	cp -r include/jdx /usr/local/include
	cp -r lib/libjdx.a lib/libjdx.$(DYN_EXT) /usr/local/lib

uninstall:
	rm -rf /usr/local/include/jdx
	rm -f /usr/local/lib/libjdx.a /usr/local/lib/libjdx.$(DYN_EXT)

jdx-static: $(OBJS)
	@mkdir -p lib
	@ar -r lib/libjdx.a $^

jdx-dynamic: $(OBJS)
	@mkdir -p lib
	$(CC) -shared -fpic $^ -o lib/libjdx.$(DYN_EXT)

build/%_c.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@rm -rf build lib