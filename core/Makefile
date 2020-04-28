CC=g++
CFLAGS=-std=c++11 -c -g -Wall -I.. -I.
CXXFLAGS=-std=c++11 -c -g -Wall -I.. -I.
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)

all: $(SOURCES) $(OBJECTS)

.cc.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS)
