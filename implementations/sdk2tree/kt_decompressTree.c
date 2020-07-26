#include <stdio.h>
#include <math.h>
#include <string.h>
#include "kTree.h"



int main(int argc, char* argv[]){

	if(argc<3){
		fprintf(stderr,"USAGE: %s <GRAPH> <list-file>\n",argv[0]);
		return(-1);
	}

	char *filename = (char *)malloc(sizeof(char)*(strlen(argv[1])+10));
	MREP * rep = loadRepresentation(argv[1]);
	
	strcpy(filename,argv[2]);
	strcat(filename,".rbfull");
	FILE *fr = fopen(filename,"w");
	fwrite(&(rep->numberOfNodes),sizeof(uint),1,fr);
	fwrite(&(rep->numberOfEdges),sizeof(ulong),1,fr);

	int * listady;
	listady = (int *) compactFullDecompression(rep);

	fwrite(listady,sizeof(int),rep->numberOfNodes+rep->numberOfEdges,fr);

	fclose(fr);
	printf("%s\n", filename);
	destroyRepresentation(rep);
	free(filename);
	free(listady);  
	return 0;
}