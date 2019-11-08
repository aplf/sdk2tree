#include <stdio.h>
#include <string.h>
#include <math.h>
#include "basic.h"

typedef struct adyList{
	uint numNodes;
    int* listady;
	ulong numEdges;    
}ALREP;

ALREP * loadAdyacencyList(char * basename);
void destroyAdyacencyList(ALREP * list);
void saveAdyacencyList(ALREP * list, char * basename);

uint adylist_getCell(ALREP * lista, uint p0, uint p1);
uint * adylist_getNeightbors(ALREP * list, uint p0);
uint * adylist_getRange(ALREP * lista, int p0, int p1, int q0, int q1);
uint * adylist_getReverseNeightbors(ALREP * lista, uint p0);

ulong * indexList(ALREP * list);