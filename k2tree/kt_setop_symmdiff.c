#include <stdio.h>
#include "kt_setOperations.h"

int main(int argc, char * argv[]){
	if(argc < 4){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}
	
	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);

	MREP * result = k2tree_symmetricdifference(repA, repB);

	if(result == NULL){
		printf("Fallo en la operación.\n");
		return -1;
	}

	destroyRepresentation(repA);
	destroyRepresentation(repB);
	saveRepresentation(result, argv[3]);
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

	//uint maximalNodes;
	uint maximalLevel = repA->maxLevel;
	
	ulong * pRepA = posByLevel(repA);
	ulong * pRepB = posByLevel(repB);
	
	ulong * minBits = (ulong *) malloc(sizeof(ulong)*(maximalLevel+1));
	if(minBits == NULL){
		printf("Error en la reserva de memoria.\n");
		return (-1);
	}
	// En el peor de los casos (en términos de espacio), la diferencia simétrica podría 
	// generar una representación completa de un grafo de k niveles, por lo que
	// se reserva todos los bits posibles en base a la cantidad de niveles.

	uint i;
	ulong auxNodes = K*K;
	for(i=0; i<=maximalLevel; i++){
		minBits[i] = auxNodes;
		//maximalNodes+=minBits[i];
		auxNodes = auxNodes * (K*K);
	}

	misBits * C = nuevoBitMap(maximalLevel+1, minBits);	
	if(C == NULL){
		printf("Error en la reserva de memoria.\n");
		return (-1);
	}

	symmetricDifferenceOperation(0u, repA, repB, pRepA, pRepB, C);
	ulong vinculolos = concatenar(C);	
	ulong numNodos = repA->numberOfNodes;

	destroyRepresentation(repA);
	destroyRepresentation(repB);

	saveOperation(C, maximalLevel, argv[3], numNodos, vinculolos);
	return 0;
}
*/