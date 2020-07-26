#include <stdio.h>
#include "NodeQueue.h"

typedef struct Queue { 
	unsigned int nodeSize; //tamaño definido para los NodeQueue de Queue
	unsigned int cant; //cantidad de elementos contenidos hasta el momento.
	NODEQUEUE * front; // puntero a primer nodo de la cola
	NODEQUEUE * rear;  // puntero al último nodo de la cola
} QUEUE;

QUEUE * newQueue(unsigned int nodeSize); // Crea una nueva cola.
void Insert(QUEUE *q, ENTRY e); // Inserta un objeto a la cola
ENTRY Delete(QUEUE *q); // elimina y devuelve un objeto de la cola
int size(QUEUE *q);