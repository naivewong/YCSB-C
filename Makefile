CC=g++
CFLAGS=-std=c++11 -g -Wall -pthread
CXXFLAGS=-std=c++11 -g -Wall -pthread -I. -I..
INCLUDES=-I./
LDFLAGS= -lpthread -ltbb -lhiredis
SUBDIRS=core db redis third-party/HdrHistogram_c
SUBSRCS=$(wildcard core/*.cc) $(wildcard db/*.cc)
OBJECTS=$(SUBSRCS:.cc=.o)

HDR_LIB=./third-party/HdrHistogram_c/hdr_lib.a
HDR_INCLUDES=-I./third-party/HdrHistogram_c/src
HDR_LDFLAGS=-lz -lm
HDR_CFLAGS=-Wall -Wno-unknown-pragmas -Wextra -Wshadow -Winit-self -Wpedantic -D_GNU_SOURCE -fPIC

.PHONY: measurements_test dependency

all: dependency ycsbc

dependency: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

ycsbc: ycsbc.cc $(OBJECTS) $(HDR_LIB)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(HDR_LDFLAGS) $(INCLUDES) $(HDR_INCLUDES) -o $@

hashmap_test: hashmap_test.cc $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LDFLAGS) -o $@

measurements_test: measurements_test.cc $(OBJECTS) $(HDR_LIB)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(HDR_LDFLAGS) $(INCLUDES) $(HDR_INCLUDES) -o $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) ycsbc

.PHONY: $(SUBDIRS) ycsbc

