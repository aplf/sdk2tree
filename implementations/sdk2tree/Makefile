CC=gcc

OBJECTS=basic.o bitrankw32int.o misBits.o adylist.o adylist_setOperations.o\
		pair.o kTree.o Entry.o NodeQueue.o Queue.o kt_setOperations.o

BINS=kt_decompressTree kt_buildTree kt_toList kt_fromList\
		kt_setop_union kt_setop_intersection kt_setop_difference kt_setop_symmdiff kt_setop_complement\
		kt_getCell kt_getNeightbors kt_getReverseNeightbors kt_getRange\
		adylist_setop_union adylist_setop_intersection adylist_setop_difference adylist_setop_symmdiff adylist_setop_complement\
		project

CCFLAGS=-Wall -O9 -DNDEBUG

# How to check the invoking target when calling make:
# https://www.gnu.org/software/make/manual/html_node/Goals.html
ifeq ($(MAKECMDGOALS),debug)
CCFLAGS=-Wall -g -DDEBUGGING
endif

DEST=.

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

all: clean bin

debug: clean bin

bin: $(OBJECTS) $(BINS)

project: $(OBJECTS) main.c
	$(CC) $(CCFLAGS) -o $(DEST)/project main.c $(OBJECTS) -lm

adylist_setop_union:
	$(CC) $(CCFLAGS) -o $(DEST)/adylist_setop_union adylist_setop_union.c $(OBJECTS) -lm

adylist_setop_intersection:
	$(CC) $(CCFLAGS) -o $(DEST)/adylist_setop_intersection adylist_setop_intersection.c $(OBJECTS) -lm

adylist_setop_difference:
	$(CC) $(CCFLAGS) -o $(DEST)/adylist_setop_difference adylist_setop_difference.c $(OBJECTS) -lm

adylist_setop_symmdiff:
	$(CC) $(CCFLAGS) -o $(DEST)/adylist_setop_symmdiff adylist_setop_symmdiff.c $(OBJECTS) -lm

adylist_setop_complement:
	$(CC) $(CCFLAGS) -o $(DEST)/adylist_setop_complement adylist_setop_complement.c $(OBJECTS) -lm

kt_decompressTree:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_decompressTree kt_decompressTree.c $(OBJECTS) -lm

kt_fromList:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_fromList kt_fromList.c $(OBJECTS) -lm

kt_toList:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_toList kt_toList.c $(OBJECTS) -lm

kt_buildTree:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_buildTree kt_buildTree.c $(OBJECTS) -lm

kt_setop_union:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_setop_union kt_setop_union.c $(OBJECTS) -lm

kt_setop_difference:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_setop_difference kt_setop_difference.c $(OBJECTS) -lm

kt_setop_intersection:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_setop_intersection kt_setop_intersection.c $(OBJECTS) -lm

kt_setop_symmdiff: 
	$(CC) $(CCFLAGS) -o $(DEST)/kt_setop_symmdiff kt_setop_symmdiff.c $(OBJECTS) -lm

kt_setop_complement:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_setop_complement kt_setop_complement.c $(OBJECTS) -lm

kt_getCell:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_getCell kt_getCell.c $(OBJECTS) -lm

kt_getNeightbors:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_getNeightbors kt_getNeightbors.c $(OBJECTS) -lm

kt_getReverseNeightbors:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_getReverseNeightbors kt_getReverseNeightbors.c $(OBJECTS) -lm

kt_getRange:
	$(CC) $(CCFLAGS) -o $(DEST)/kt_getRange kt_getRange.c $(OBJECTS) -lm

clean:
	rm -f $(OBJECTS) $(BINS)
	cd $(DEST); rm -f *.a $(BINS)
