#include <stdio.h>
#include "kTree.h"
#include "adylist.h"



int main(int argc, char* argv[]){
	if(argc<3){
		fprintf(stderr,"USAGE: %s <GRAPH> <list-file>\n",argv[0]);
		return(-1);
	}

	MREP * rep = loadRepresentation(argv[1]);	
	int * listady;
	listady = (int *) compactFullDecompression(rep);

	ALREP * adyList = (ALREP *) malloc(sizeof(struct adyList));
	adyList->numNodes = rep->numberOfNodes;
	adyList->numEdges = rep->numberOfEdges;
	adyList->listady = listady;

	saveAdyacencyList(adyList, argv[2]);
	
	destroyAdyacencyList(adyList);
	destroyRepresentation(rep);
	return 0;
}