CC=g++
CFLAGS=-std=c++11 -c -g -Wall -fgnu-tm -I.. -I.
CXXFLAGS=-std=c++11 -c -g -Wall -fgnu-tm -I.. -I. -I./rocksdb-cloud/include
INCLUDES=-I../
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)

TMPVAR := $(OBJECTS)
OBJECTS = $(filter-out rocksdb_db.o rocksdb_cloud_db.o db_factory.o, $(TMPVAR))

all: $(SOURCES) $(OBJECTS)

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	$(RM) $(OBJECTS)

