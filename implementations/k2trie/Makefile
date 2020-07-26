CPP=g++

OBJECTS=treeBlock.o

BINS=project

CPPFLAGS=-Wall -O9 -DNDEBUG

# How to check the invoking target when calling make:
# https://www.gnu.org/software/make/manual/html_node/Goals.html
ifeq ($(MAKECMDGOALS),debug)
CPPFLAGS=-Wall -g -DDEBUGGING
endif

DEST=.

%.o: %.c
	$(CPP) $(CPPFLAGS) -c $< -o $@

all: clean bin

debug: clean bin

bin: $(OBJECTS) $(BINS)

project: 
	$(CPP) $(CPPFLAGS) -o $(DEST)/project main.cpp $(OBJECTS) -lm

clean:
	rm -f project *.o
