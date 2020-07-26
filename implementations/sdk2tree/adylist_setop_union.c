#include <stdio.h>
#include "adylist_setOperations.h"

int main(int argc, char* argv[]){

	if(argc<4){
		fprintf(stderr,"USAGE: %s <LISTADY> <LISTADY> <LISTADY>\n",argv[0]);
		return(-1);
	}

	// Cargando Lista de Adyacencia A
	ALREP * A = loadAdyacencyList(argv[1]);
	// Cargando Lista de Adyacencia B
	ALREP * B = loadAdyacencyList(argv[2]);

	ALREP * listadyResult = adylistUnionOperation(A, B);
	if(listadyResult == NULL){
		printf("Error en la operación...\n");
		return 0;
	}

	destroyAdyacencyList(A);
	destroyAdyacencyList(B);
	// Desplegando Resultado de la Unión
	//saveAdyList(edgesResult[0], nodes, listadyResult, argv[3]);
	saveAdyacencyList(listadyResult, argv[3]);
	destroyAdyacencyList(listadyResult);

	return 0;
}

