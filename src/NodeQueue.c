#include <stdio.h>
#include "NodeQueue.h" 
#include <stdlib.h>


NODEQUEUE * newNodeQueue(unsigned char s) { 
	NODEQUEUE *n; 
	n = (NODEQUEUE *) malloc(sizeof(NODEQUEUE));
	n->t=s; 
	n->r=0; 
	n->f=0;
	n->u=0;
	n->e=(ENTRY *)malloc( n->t*sizeof(ENTRY));
	n->next = NULL;
	return n; 
} 

int insertarEntry(NODEQUEUE * n, ENTRY nuevo){
	if(estaLleno(n)){
		return 0;
	}

	if(estaVacio(n)){
		n->r = 0;
		n->f = 0;
	}else if(n->r +1 == n->t){
		n->r = 0;
	}else{
		n->r = n->r +1;
	}

	n->e[n->r] = nuevo;
	n->u = n->u +1;
	
	return 1;
}

ENTRY extraerEntry(NODEQUEUE * n){
	ENTRY retorno = n->e[n->f];

	if(n->f +1 == n->t){
		n->f = 0;
	}else{
		n->f = n->f +1;
	}
	n->u = n->u -1;
		
	return retorno;
}

unsigned char estaLleno(NODEQUEUE *n) { 
	return (unsigned char) (n->u == n->t);
}

unsigned char estaVacio(NODEQUEUE *n) { 
	return (unsigned char) (n->u == 0);
}

void destroyNodeQueue(NODEQUEUE *n){
	free(n->e);
	free(n);
	return;
}
