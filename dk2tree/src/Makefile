CACHE=mem


C=gcc

#MACHINE_FLAGS=-mpopcnt

ifeq ($(CACHE),mem)
OBJECTS=basic.o encoding.o util.o nodeManager-mem-complete.o k2tree-common.o k2tree-operations.o k2tree-impl.o hash.o MemoryManager.o vocUtils.o
else
OBJECTS=basic.o encoding.o util.o nodeManager-disk-complete.o k2tree-common.o k2tree-operations.o k2tree-impl.o hash.o MemoryManager.o vocUtils.o
endif

BINS=testGraph testRebuild testRange project
BINS_NAMES=load graph loadfile insert neighbors rebuild optimize project range

K2TREE=k2tree.a

ifeq ($(CACHE), mem)
    DEFS=-DMEMCACHE
else
    DEFS=-DRANDOMCACHE
endif


CFLAGS=-Wall -O9 -g3 $(MACHINE_FLAGS) $(DEFS) $(CUSTOM) -DNDEBUG

%.o: %.c
	$(C) $(CFLAGS) -c $< -o $@

all: bin

dopt: CFLAGS=-Wall -g3 -O9 -DNDEBUG $(MACHINE_FLAGS) $(DEFS) $(CUSTOM) 
dopt: bin

debug: CFLAGS=-Wall -g3 -DDEBUGGING $(MACHINE_FLAGS) $(DEFS) $(CUSTOM) #-std=gnu89 #-std=gnu11
debug: bin


archive : $(OBJECTS) 
	ar rc $(K2TREE) $(OBJECTS) 

bin: archive $(BINS)

project: archive
	gcc $(CFLAGS) -o project main.c $(K2TREE) -lm

unoptimized: archive
	gcc -Wall -g3 $(MACHINE_FLAGS) $(DEFS) $(CUSTOM) -o unoptimized main.c $(K2TREE) -lm

testGraph: archive
	gcc $(CFLAGS) -o graph testCreateFromGraph.c $(K2TREE) -lm

testLoad: archive
	gcc $(CFLAGS) -o load testLoad.c $(K2TREE) -lm
	
testLoadFile: archive
	gcc $(CFLAGS) -o loadfile testLoadFromFile.c $(K2TREE) -lm

testInsert: archive
	gcc $(CFLAGS) -o insert testInsert.c $(K2TREE) -lm
	
testNeighbors: archive
	gcc $(CFLAGS) -o neighbors testNeighbors.c $(K2TREE) -lm
	
testRebuild: archive
	gcc $(CFLAGS) -o rebuild testRebuild.c $(K2TREE) -lm
	
testRange: archive
	gcc $(CFLAGS) -o range testRange.c $(K2TREE) -lm

clean:
	rm -f *.o *.a $(BINS_NAMES) 
