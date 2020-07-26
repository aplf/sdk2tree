#include <stdio.h>
#include <stdlib.h>
#include "kTree.h"

int main(int argc, char * argv[]){
	setbuf(stdin, NULL);
	if(argc<4){
		fprintf(stderr,"USAGE: %s <GRAPH> <id_objectFrom> <id_objectTo>\n",argv[0]);
		return(-1);
	}

	MREP * rep = loadRepresentation(argv[1]);	

	int idOrigen = atoi(argv[2]);
	int idDestino = atoi(argv[3]);

	uint resp = compact2CheckLinkQuery(rep, idOrigen, idDestino);
	printf("%u\n", resp);

	destroyRepresentation(rep);

	return 0;
}