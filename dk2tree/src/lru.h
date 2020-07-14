#include "util.h"

#define POS_HEAD (-1)
#define POS_NONE (-2)

typedef struct sLRUList {
	int *next;
	int *prev;
	int lru;
} LRUList;

LRUList * initLRU(int size);

void extractPositionLRU(LRUList * list, int pos);

int popLRU(LRUList * list);

void pushLRU(LRUList * list, int pos);

