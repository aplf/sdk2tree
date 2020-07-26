CC=gcc

#MACHINE_FLAGS=-mpopcnt

CACHE=mem

BINS=testGraph testLoad testLoadFile testInsert testNeighbors testRebuild testRange project

K2TREE=k2tree.a

ifeq ($(CACHE),mem)
    OBJECTS=basic.o encoding.o util.o nodeManager-mem-complete.o k2tree-common.o k2tree-operations.o k2tree-impl.o hash.o MemoryManager.o vocUtils.o
	DEFS=-DMEMCACHE
else
    OBJECTS=basic.o encoding.o util.o nodeManager-disk-complete.o k2tree-common.o k2tree-operations.o k2tree-impl.o hash.o MemoryManager.o vocUtils.o
	DEFS=-DRANDOMCACHE
endif

# ifeq ($(CACHE), mem)
#     DEFS=-DMEMCACHE
# else
#     DEFS=-DRANDOMCACHE
# endif

CCFLAGS=-Wall -O9 $(MACHINE_FLAGS) $(DEFS) $(CUSTOM) -DNDEBUG

# How to check the invoking target when calling make:
# https://www.gnu.org/software/make/manual/html_node/Goals.html
ifeq ($(MAKECMDGOALS),debug)
CCFLAGS=-Wall -g $(MACHINE_FLAGS) $(DEFS) $(CUSTOM) -DDEBUGGING #-std=gnu89 #-std=gnu11
endif


%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

all: clean bin

debug: clean bin

archive: $(OBJECTS) 
	ar rc $(K2TREE) $(OBJECTS) 

bin: archive $(BINS)

project: archive
	$(CC) $(CCFLAGS) -o project main.c $(K2TREE) -lm

testGraph: archive
	$(CC) $(CCFLAGS) -o testGraph testCreateFromGraph.c $(K2TREE) -lm

testLoad: archive
	$(CC) $(CCFLAGS) -o testLoad testLoad.c $(K2TREE) -lm
	
testLoadFile: archive
	$(CC) $(CCFLAGS) -o testLoadFile testLoadFromFile.c $(K2TREE) -lm

testInsert: archive
	$(CC) $(CCFLAGS) -o testInsert testInsert.c $(K2TREE) -lm
	
testNeighbors: archive
	$(CC) $(CCFLAGS) -o testNeighbors testNeighbors.c $(K2TREE) -lm
	
testRebuild: archive
	$(CC) $(CCFLAGS) -o testRebuild testRebuild.c $(K2TREE) -lm
	
testRange: archive
	$(CC) $(CCFLAGS) -o testRange testRange.c $(K2TREE) -lm

clean:
	rm -f *.o *.a $(BINS) $(OBJECTS)
