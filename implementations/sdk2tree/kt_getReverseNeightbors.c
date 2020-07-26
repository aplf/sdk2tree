#include <stdio.h>
#include <stdlib.h>
#include "kTree.h"

int main(int argc, char * argv[]){
	setbuf(stdin, NULL);
	if(argc<3){
		fprintf(stderr,"USAGE: %s <GRAPH> <id_object>\n",argv[0]);
		return(-1);
	}
	MREP * rep = loadRepresentation(argv[1]);	
	uint i;
	int idNodo = atoi(argv[2]);
	uint * listady;
	listady = compactInverseList(rep, idNodo);
	printf("(%u)\t", listady[0]);
	for(i=0; i<listady[0]; i++){
		printf("%u\t", listady[i+1]);
	}
	printf("\n");

	destroyRepresentation(rep);

	return 0;
}