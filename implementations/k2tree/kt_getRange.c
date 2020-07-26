#include <stdio.h>
#include <stdlib.h>
#include "kTree.h"

int main(int argc, char * argv[]){
	setbuf(stdin, NULL);
	if(argc<6){
		fprintf(stderr,"USAGE: %s <GRAPH> <x0> <x1> <y0> <y1>\n",argv[0]);
		return(-1);
	}

	MREP * rep = loadRepresentation(argv[1]);	

	uint p1=atoi(argv[2]);
	uint p2=atoi(argv[3]);
	uint q1=atoi(argv[4]);
	uint q2=atoi(argv[5]);

	uint ** respuesta = compactRangeQuery(rep, p1, p2, q1, q2);

	printf("Range: [%d,%d]-[%d,%d], total of links %d\n",p1,p2,q1,q2,respuesta[0][0]);
	uint i = respuesta[0][0];
	for(i=0;i<respuesta[0][0];i++){
  		printf("(%d,%d)\t",respuesta[0][i+1],respuesta[1][i+1]);
  	}
	printf("\n");


	destroyRepresentation(rep);

	return 0;
}