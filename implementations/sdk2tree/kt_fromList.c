#include <stdio.h>
#include <math.h>
#include "kTree.h"
#include "adylist.h"


int main(int argc, char* argv[]){
	uint nodes; 
	ulong edges;
	register ulong i;
	
	if(argc<3){
		fprintf(stderr,"USAGE: %s <GRAPH> <outfile>\n",argv[0]);
		return(-1);
	}

	
	ALREP * lista = (ALREP *) loadAdyacencyList(argv[1]);
	nodes = lista->numNodes;
	edges = lista->numEdges;

	uint max_level = floor(log(nodes)/log(K)); 
	if(floor(log(nodes)/log(K))==(log(nodes)/log(K))) {
		max_level=max_level-1;
	}
	
	uint nodes_read=0;
	uint *xedges = (uint *)malloc(sizeof(uint)*edges);
	uint *yedges = (uint *)malloc(sizeof(uint)*edges);
	uint cedg = 0;
	int k;
	for(i=0;i<nodes+edges;i++) {
		k = lista->listady[i];
		if(k<0) {
			nodes_read++;
		}
		else {
			k--;
			xedges[cedg]=nodes_read-1;
			yedges[cedg]=k;
			cedg++;
		}
	}
	
	MREP * rep;
 	
 	rep = compactCreateKTree(xedges, yedges, nodes,edges,max_level);
	saveRepresentation(rep,argv[2]);	
	
	destroyAdyacencyList(lista);
	destroyRepresentation(rep);
	
	free(xedges);
	free(yedges);

	return 0;
}


