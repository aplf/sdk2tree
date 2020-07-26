#include <stdio.h>
#include "kt_setOperations.h"

int main(int argc, char * argv[]){
	if(argc < 3){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}

/*
	setbuf(stdout, NULL);
*/
	
	MREP * repA = loadRepresentation(argv[1]);
	MREP * result = k2tree_complement(repA);

	if(result == NULL){
		printf("Fallo en la operación.\n");
		return -1;
	}

	destroyRepresentation(repA);
	saveRepresentation(result, argv[2]);
	destroyRepresentation(result);
	return 0;
}

/*


int main(int argc, char * argv[]){
	if(argc < 3){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}

// BBOORRAARR
	setbuf(stdout, NULL);
// BBOORRAARR
	
	MREP * repA = loadRepresentation(argv[1]);

	//uint maximalNodes;
	uint maximalLevel = repA->maxLevel;

	ulong * pRepA = posByLevel(repA);

	ulong * minBits = (ulong *) malloc(sizeof(ulong) * (maximalLevel+1));
	if(minBits == NULL){
		printf("Error en la reserva de memoria.\n");
		return (-1);
	}
	// Para el complemento, el peor caso es generar como resultado de la
	// operación una representación que no compacte sectores
	// Por lo que se reserva el espacio necesario para una representación completa
	
	ulong i, auxNodes = K*K;
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

//	Variable para calcular el número de vínculos dentro de la operación.
	ulong limiteSuperior = repA->div_level_table[0] * 2 - 1;
	ulong limits[4] = {0, limiteSuperior, 0, limiteSuperior};

	complementOperation(0u, repA, pRepA, C, limits);
	ulong vinculolos = concatenar(C);
	ulong numNodos = repA->numberOfNodes;
	destroyRepresentation(repA);
	saveOperation(C, maximalLevel, argv[2], numNodos, vinculolos);
	return 0;
}
*/