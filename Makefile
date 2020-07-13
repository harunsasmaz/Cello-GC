CC ?= gcc
AR ?= ar
CFLAGS = -ansi -O3 -fpic -pedantic -g -Wall -Wno-unused
LFLAGS = -fpic
ECFLAGS = -std=c99 -O3 -g

INCDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib

OBJECT = collector.o
STATIC = libgc.a
DYNAMIC = libgc.so

all: $(STATIC) $(DYNAMIC)

$(OBJECT): collector.c collector.h
	$(CC) -c $(CFLAGS) collector.c

$(DYNAMIC): $(OBJECT)
	$(CC) -shared -o $@ $^

$(STATIC): $(OBJECT)
	$(AR) rcs $@ $^

install:
	cp -f $(STATIC) $(LIBDIR)
	cp -f collector.h $(INCDIR)

check:
	$(CC) $(ECFLAGS) test/simple.c collector.c -o ./test/simple && \
	./test/simple
	$(CC) $(ECFLAGS) test/intermediate.c collector.c -o ./test/intermediate && \
	./test/intermediate  

clean:
	rm -rf $(STATIC) $(DYNAMIC) $(OBJECT)
	rm -rf test/simple.dSYM test/intermediate.dSYM
	rm -f test/intermediate test/simple