CPP=gcc

OBJECTS=basic.o bitrankw32int.o misBits.o adylist.o adylist_setOperations.o\
		pair.o kTree.o Entry.o NodeQueue.o Queue.o kt_setOperations.o

BINS=project

CPPFLAGS=-Wall -O3 -g -DNDEBUG

# How to check the invoking target when calling make:
# https://www.gnu.org/software/make/manual/html_node/Goals.html
ifeq ($(MAKECMDGOALS),debug)
CPPFLAGS=-Wall -g -DDEBUGGING
endif

#CPPFLAGS=-Wall -g 
DEST=.

%.o: %.c
	$(CPP) $(CPPFLAGS) -c $< -o $@

debug: clean bin

all: clean bin

bin: $(OBJECTS) $(BINS)

project: $(OBJECTS) main.c
	gcc $(CPPFLAGS) -o $(DEST)/project main.c $(OBJECTS) -lm

clean:
	rm -f $(OBJECTS) $(BINS)
	cd $(DEST); rm -f *.a $(BINS)
