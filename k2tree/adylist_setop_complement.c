#include <stdio.h>
#include "adylist_setOperations.h"

int main(int argc, char* argv[]){
	if(argc<3){
		fprintf(stderr,"USAGE: %s <LISTADY> <LISTADY>\n",argv[0]);
		return(-1);
	}
/*
	char *filename = (char *) malloc(sizeof(char)*(strlen(argv[1])+8));
	strcpy(filename,argv[1]);
	strcat(filename,".rbfull");
	FILE * ft = fopen(filename,"r");
	uint nodes;
	fread(&nodes,sizeof(uint),1,ft);
	ulong edges;
	fread(&edges,sizeof(ulong),1,ft);
	int* adyList = (int*)malloc(sizeof(int)*(edges+nodes));
	fread(adyList,sizeof(int), nodes+edges,ft);
	fclose(ft);
	free(filename);
*/

	ALREP * A = loadAdyacencyList(argv[1]);

	// Desplegando Lista de Adyacencia
/*
	printf("Lista de Adyacencia de %s\n", argv[1]);
	printf("Nodes: %d\n", nodes);
	printf("Edges: %ld\n", edges);
	ulong i;
	for(i=0; i<edges+nodes; i++){
		if(adyList[i] < 0){
			printf("\n");
		}
		printf("%d\t", adyList[i]);
	}
	printf("\n");
*/
	// Operación de Complemento sobre la lista
	ALREP * listadyResult = adylistComplementOperation(A);
	if(listadyResult == NULL){
		printf("Error en la operación...\n");
		return 0;
	}

	destroyAdyacencyList(A);

	// Desplegando Resultado de la Diferencia Simétrica
/*
	printf("Lista de Adyacencia del Complemento\n");
	printf("Nodes: %d\n", nodes);
	printf("Edges: %ld\n", edgesResult[0]);
	for(i=0; i<edgesResult[0]+nodes; i++){
		if(listadyResult[i] < 0){
			printf("\n");
		}
		printf("%d\t", listadyResult[i]);
	}
*/
	saveAdyacencyList(listadyResult, argv[2]);

	destroyAdyacencyList(listadyResult);
/*
	printf("\n");
	printf("Fin Ejecución...\n");
*/
	return 0;
}



