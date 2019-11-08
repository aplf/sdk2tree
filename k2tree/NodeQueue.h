#include <stdio.h>
#include "Entry.h"

typedef struct NodeQueue {
	unsigned char t; // Tamaño del nodo
	unsigned char f; // front dentro del nodo 
	unsigned char r; // rear dentro del nodo
	unsigned char u; // numero de elementos en el nodo
	ENTRY *e; 
	struct NodeQueue * next;
} NODEQUEUE;

NODEQUEUE * newNodeQueue(unsigned char size);// Crea un nuevo nodo
int insertarEntry(NODEQUEUE * n, ENTRY nuevo); // Inserta un nuevo ENTRY en el nodo. Si retorna 0 falló la inserción.
ENTRY extraerEntry(NODEQUEUE * n);
unsigned char estaLleno(NODEQUEUE *n);// Verifica si un nodo está lleno
unsigned char estaVacio(NODEQUEUE *n);// y si un nodo esta vacio
void destroyNodeQueue(NODEQUEUE *n); //Elimina el NodeQueue