#include <stdio.h>
#include "kt_setOperations.h"

int main(int argc, char * argv[]){
	if(argc < 4){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);
	MREP * result = k2tree_difference(repA, repB);

	if(result == NULL){
		printf("Fallo en la operación.\n");
		return -1;
	}

	destroyRepresentation(repA);
	destroyRepresentation(repB);
	saveRepresentation(result, argv[3]);
	destroyRepresentation(result);
	return 0;
}

/*
RESPALDO... PREVIO A LA IMPLEMENTACIÓN DE LAS FUNCIONES CON DOS/UN PARAMETROS Y RETORNAN MREP

int main(int argc, char * argv[]){
	if(argc < 4){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);

	ulong maximalBits = repA->btl_len;
	uint maximalLevel = repA->maxLevel;
	
	ulong * pRepA = posByLevel(repA);
	ulong * pRepB = posByLevel(repB);
	
	ulong * minBits = (ulong *) malloc(sizeof(ulong)*(maximalLevel+1));
	if(minBits == NULL){
		printf("Error en la reserva de memoria.\n");
		return (-1);
	}
	// En la diferencia, la reserva de espacio debe ser suficiente para almacenar
	// la representación completa del primer k2-tree, ya que podrían no tener
	// elementos en común.
	uint i;
	for(i=0; i<=maximalLevel; i++){
		minBits[i] = pRepA[i+1] - pRepA[i];
	}
	minBits[maximalLevel] = maximalBits - pRepA[maximalLevel];

	misBits * C = nuevoBitMap(maximalLevel+1, minBits);	
	if(C == NULL){
		printf("Error en la reserva de memoria.\n");
		return (-1);
	}
	
	differenceOperation(0u, repA, repB, pRepA, pRepB, C);
	ulong vinculolos = concatenar(C);	
	ulong numNodos = repA->numberOfNodes;
	
	destroyRepresentation(repA);
	destroyRepresentation(repB);

	saveOperation(C, maximalLevel, argv[3], numNodos, vinculolos);
	return 0;
}
*/