#include <stdio.h>
#include "Queue.h"
#include <stdlib.h>

/**************************************/
QUEUE * newQueue(unsigned int nodSiz) { 
  QUEUE *q = (QUEUE *) malloc(sizeof(QUEUE)) ;
  q->nodeSize = nodSiz; 
  q->cant = 0;
  q->front = newNodeQueue(nodSiz);
  q->rear = q->front;    
  return q;
} 

/************************************/
void Insert(QUEUE *q, ENTRY ent) { 
  if(estaLleno(q->rear)) {
    //El Nodo rear esta lleno 
    q->rear->next = newNodeQueue(q->nodeSize); 
    q->rear = q->rear->next;
  }
  if(insertarEntry(q->rear, ent));
  q->cant ++;
}

/*******************************/
ENTRY Delete(QUEUE *q) {
  ENTRY ee; 
  if(q->cant > 0){
    if(estaVacio(q->front)){ //El NodeQueue front está vacío y se debe trasladar
      NODEQUEUE* siguiente = q->front->next;
      destroyNodeQueue(q->front);
      q->front = siguiente;
    }
    ee = extraerEntry(q->front);
    q->cant --;
  }
  return ee;  
}

/********************************/
int size(QUEUE *q) { 
  int sz =0;
  NODEQUEUE *n;
  for (n=q->front; n!=NULL;n=n->next){ 
    sz++;
  }
  return sz;
}

