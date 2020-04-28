CC=g++
CFLAGS=-std=c++11 -c -g -Wall -fgnu-tm -I.. -I.
CXXFLAGS=-std=c++11 -c -g -Wall -fgnu-tm -I.. -I.
INCLUDES=-I../
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)

all: $(SOURCES) $(OBJECTS)

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	$(RM) $(OBJECTS)

