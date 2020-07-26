#include <stdio.h>
#include "kt_setOperations.h"

int main(int argc, char * argv[]){
	if(argc < 4){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}

/*
	setbuf(stdout, NULL);
*/

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);
	MREP * result = k2tree_intersection(repA, repB);

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

// BBOORRAARR
	setbuf(stdout, NULL);
// BBOORRAARR

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);
	uint maximalLevel = repA->maxLevel;
	
	ulong * pRepA = posByLevel(repA);
	ulong * pRepB = posByLevel(repB);
	
	ulong * minBits = (ulong *) malloc(sizeof(ulong)*(maximalLevel+1));
	if(minBits == NULL){
		printf("Se ha generado un problema...\n");
		return 0;
	}
	// La intersección podría tener a lo más, tantas celdas activas 
	// como las que posee el menor de los dos elementos operados.

	ulong numBitsByLevelA, numBitsByLevelB;
	uint i;
	for(i = 0; i <= maximalLevel; i++){
//		printf("%d/%d\t", i,maximalLevel);
		if(i==repA->maxLevel){
//			printf("a  \t");
			numBitsByLevelA = repA->btl_len - pRepA[i];
			numBitsByLevelB = repB->btl_len - pRepB[i];
//			printf("b  \n");
		}else{
//			printf("A  \t");
			numBitsByLevelA = pRepA[i+1] - pRepA[i];
			numBitsByLevelB = pRepB[i+1] - pRepB[i];
//			printf("B  \n");
		}
//			printf("X  \t");
//		printf("minBitsA: %d\t\tminBitsB: %d\t\t", numBitsByLevelA, numBitsByLevelB);
		minBits[i] = (numBitsByLevelA > numBitsByLevelB)?numBitsByLevelB:numBitsByLevelA;
//			printf("X  \t");
//		printf("minBits[%d] = %d\n", i, minBits[i]);
//		printf("-------\n");
	}

//	printf("nuevoBitMap inicio\n");
	misBits * C = nuevoBitMap(maximalLevel+1, minBits);
	if(C == NULL){
		printf("Error en la reserva de memoria.\n");
		return (-1);
	}
	intersectionOperation(0u, repA, repB, pRepA, pRepB, C);
	ulong vinculolos = concatenar(C);	
	ulong numNodos = repA->numberOfNodes;
	
	destroyRepresentation(repA);
	destroyRepresentation(repB);

	saveOperation(C, maximalLevel, argv[3], numNodos, vinculolos);
	return 0;
}
*/