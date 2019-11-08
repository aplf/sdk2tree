#include <stdio.h>
#include "adylist_setOperations.h"

int main(int argc, char* argv[]){

	if(argc<4){
		fprintf(stderr,"USAGE: %s <LISTADY> <LISTADY> <LISTADY>\n",argv[0]);
		return(-1);
	}

	// Cargando Lista de Adyacencia A
/*
	char *filename = (char *) malloc(sizeof(char)*(strlen(argv[1])+8));
	strcpy(filename,argv[1]);
	strcat(filename,".rbfull");
	FILE * ft = fopen(filename,"r");
	uint nodes;
	fread(&nodes,sizeof(uint),1,ft);
	ulong edgesA;
	fread(&edgesA,sizeof(ulong),1,ft);
	int* listadyA = (int*)malloc(sizeof(int)*(edgesA+nodes));
	fread(listadyA,sizeof(int), nodes+edgesA,ft);
	fclose(ft);
	free(filename);
*/
	ALREP * A = loadAdyacencyList(argv[1]);

	// Cargando Lista de Adyacencia B
/*
	filename = (char *) malloc(sizeof(char)*(strlen(argv[1])+8));
	strcpy(filename,argv[2]);
	strcat(filename,".rbfull");
	ft = fopen(filename,"r");
	uint nodesB;
	fread(&nodesB,sizeof(uint),1,ft);
	ulong edgesB;
	fread(&edgesB,sizeof(ulong),1,ft);
	int* listadyB = (int*)malloc(sizeof(int)*(edgesB+nodesB));
	fread(listadyB,sizeof(int), nodesB+edgesB,ft);
	fclose(ft);
	free(filename);
*/
	ALREP * B = loadAdyacencyList(argv[2]);

	// Desplegando A
/*
	printf("Lista de Adyacencia de %s\n", argv[1]);
	printf("Nodes: %d\n", nodes);
	printf("Edges: %ld\n", edgesA);
	ulong i;
	for(i=0; i<edgesA+nodes; i++){
		if(listadyA[i] < 0){
			printf("\n");
		}
		printf("%d\t", listadyA[i]);
	}
	printf("\n");
*/
	// Desplegando B
/*
	printf("Lista de Adyacencia de %s\n", argv[2]);
	printf("Nodes: %d\n", nodes);
	printf("Edges: %ld\n", edgesB);
	for(i=0; i<edgesB+nodes; i++){
		if(listadyB[i] < 0){
			printf("\n");
		}
		printf("%d\t", listadyB[i]);
	}
	printf("\n");
*/
	// Operación de Intersección entre listas de adyacencia
	ALREP * listadyResult = adylistIntersectionOperation(A, B);
	if(listadyResult == NULL){
		printf("Error en la operación...\n");
		return 0;
	}

	destroyAdyacencyList(A);
	destroyAdyacencyList(B);

	// Desplegando Resultado de la Intersección
/*
	printf("Lista de Adyacencia de la Intersección\n");
	printf("Nodes: %d\n", nodes);
	printf("Edges: %ld\n", edgesResult[0]);
	for(i=0; i<edgesResult[0]+nodes; i++){
		if(listadyResult[i] < 0){
			printf("\n");
		}
		printf("%d\t", listadyResult[i]);
	}
*/
	saveAdyacencyList(listadyResult, argv[3]);

	destroyAdyacencyList(listadyResult);
/*
	printf("\n");
	printf("Fin Ejecución...\n");
*/
	return 0;
}

