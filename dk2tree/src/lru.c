#include "lru.h"
/*
Cola para implementar LRU.
Permite eliminar un elemento (posición) calquera, 
engadir como mru e obter o lru.
Implementación: lista doblemente enlazada con operacións de cola (push, pop) e acceso directo a posicións para eliminar
Utiliza 12*size bytes (+overhead)
*/


LRUList * initLRU(int size) {
	LRUList * list = (LRUList *) malloc(sizeof(LRUList));
	list->next = (int *) malloc(size*sizeof(int));
	list->prev = (int *) malloc(size*sizeof(int));
	list->lru = 0;
	int i;
	for (i = 0; i < size-1; i++) {
		list->next[i] = i+1;
	}
	for (i = 1; i < size; i++) {
		list->prev[i] = i-1;
	}
	list->next[size-1] = 0;
	list->prev[0] = size-1;
	return list;
}

void extractPositionLRU(LRUList * list, int pos) {
	if (pos == list->lru) {
		//We are extracting the head (lru) element
		list->lru = list->next[pos];
	}
	list->next[list->prev[pos]] = list->next[pos];
	list->prev[list->next[pos]] = list->prev[pos];
}

int popLRU(LRUList * list) {
	int ret = list->lru;
	extractPositionLRU(list, list->lru);
	return ret;
}

void pushLRU(LRUList * list, int pos) {
	int oldmru = list->prev[list->lru];
	list->next[oldmru] = pos;
	list->prev[pos] = oldmru;
	list->next[pos] = list->lru;
	list->prev[list->lru] = pos;
}

void printLRU(LRUList * list) {
	int curpos = list->lru;
	do {
		printf("%2d ", curpos);
		curpos = list->next[curpos];
	} while(curpos != list->lru);
	printf("\n");
}

