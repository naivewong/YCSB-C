CC=g++
CFLAGS=-std=c++11 -g -Wall -pthread
CXXFLAGS=-std=c++11 -g -Wall -pthread -I. -I.. -I./rocksdb-cloud/include
INCLUDES=-I./
LDFLAGS= -lpthread -ltbb -lhiredis
SUBDIRS=core db redis third-party/HdrHistogram_c
SUBSRCS=$(wildcard core/*.cc) $(wildcard db/*.cc)
OBJECTS=$(SUBSRCS:.cc=.o)

TMPVAR := $(OBJECTS)
OBJECTS = $(filter-out db/rocksdb_db.o db/rocksdb_cloud_db.o db/db_factory.o, $(TMPVAR))

HDR_LIB=./third-party/HdrHistogram_c/hdr_lib.a
HDR_INCLUDES=-I./third-party/HdrHistogram_c/src
HDR_LDFLAGS=-lz -lm
HDR_CFLAGS=-Wall -Wno-unknown-pragmas -Wextra -Wshadow -Winit-self -Wpedantic -D_GNU_SOURCE -fPIC

ROCKSDB_LIB=./rocksdb-cloud/librocksdb.a
ROCKSDB_CXXFLAGS=-fno-rtti
ROCKSDB_PLATFORM_CXXFLAGS=-std=c++11 -faligned-new -DHAVE_ALIGNED_NEW -DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX -DOS_LINUX -fno-builtin-memcmp -DROCKSDB_FALLOCATE_PRESENT -DZLIB -DTBB -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_BACKTRACE -DROCKSDB_RANGESYNC_PRESENT -DROCKSDB_SCHED_GETCPU_PRESENT -march=native -DHAVE_SSE42 -DHAVE_PCLMUL -DROCKSDB_SUPPORT_THREAD_LOCAL
ROCKSDB_PLATFORM_LDFLAGS=-lpthread -lrt -lz -ltbb
ROCKSDB_EXEC_LDFLAGS=-ldl -lpthread
ROCKSDB_INCLUDES=-I./rocksdb-cloud/include

ROCKSDB_CLOUD_PLATFORM_CXXFLAGS=-std=c++11 -faligned-new -DHAVE_ALIGNED_NEW -DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX -DOS_LINUX -fno-builtin-memcmp -DROCKSDB_FALLOCATE_PRESENT -DSNAPPY -DGFLAGS=1 -DZLIB -DBZIP2 -DLZ4 -DZSTD -DTBB -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_BACKTRACE -DROCKSDB_RANGESYNC_PRESENT -DROCKSDB_SCHED_GETCPU_PRESENT -I/usr/local/include/ -DUSE_AWS -march=native -DHAVE_SSE42 -DHAVE_PCLMUL -DROCKSDB_SUPPORT_THREAD_LOCAL
ROCKSDB_CLOUD_LDFLAGS=-laws-cpp-sdk-s3 -laws-cpp-sdk-kinesis -laws-cpp-sdk-core -laws-cpp-sdk-transfer -lpthread -lrt -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd -ltbb -ldl -lpthread

CXXFLAGS+=$(HDR_INCLUDES)
CXXFLAGS+=$(ROCKSDB_INCLUDES)

.PHONY: measurements_test dependency ycsbc

all: dependency ycsbc

dependency: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

ycsbc: ycsbc.cc db/rocksdb_db.cc db/rocksdb_cloud_db.cc db/db_factory.cc $(OBJECTS) $(HDR_LIB) $(ROCKSDB_LIB)
	$(CC) $(CFLAGS) $^ -O2 $(LDFLAGS) $(HDR_LDFLAGS) $(ROCKSDB_PLATFORM_LDFLAGS) $(INCLUDES) $(HDR_INCLUDES) $(ROCKSDB_INCLUDES) $(ROCKSDB_CXXFLAGS) $(ROCKSDB_PLATFORM_CXXFLAGS) $(ROCKSDB_CLOUD_PLATFORM_CXXFLAGS) $(ROCKSDB_EXEC_LDFLAGS) $(ROCKSDB_CLOUD_LDFLAGS) -o $@

measurements_test: measurements_test.cc $(OBJECTS) $(HDR_LIB)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(HDR_LDFLAGS) $(INCLUDES) $(HDR_INCLUDES) -o $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) ycsbc

.PHONY: $(SUBDIRS) ycsbc

