#include <stdio.h>
#include "kt_setOperations.h"

int main(int argc, char * argv[]){
	if(argc < 4){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);
	MREP * result = k2tree_union(repA, repB);

	if(result == NULL){
		printf("Operación de unión finalizada..\n");
		return -1;
	}

	saveRepresentation(result, argv[3]);
	destroyRepresentation(repA);
	destroyRepresentation(repB);
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

	uint maximalLevel = repA->maxLevel;

	ulong i = 0, maximalBits = 0, calculador = 1; 
	
	for(i = 0; i <= maximalLevel; i++){
		calculador *= (K*K);
		maximalBits += calculador;
	}

	ulong minBits[] = {maximalBits};
	// En la unión, el peor de los casos corresponde a generar una 
	// representación completa de un k2-tree, sin sectores en 0.

	misBits * C = nuevoBitMap(1u, minBits);
	if(C == NULL){
		printf("Error en la reserva de memoria.\n");
		return (-1);
	}

	ulong numEdges = unionOperation(repA, repB, C);
	concatenar(C);	
	ulong numNodos = repA->numberOfNodes;

	destroyRepresentation(repA);
	destroyRepresentation(repB);

	saveOperation(C, maximalLevel, argv[3], numNodos, numEdges);
	return 0;
}
*/