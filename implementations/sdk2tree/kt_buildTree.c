#include <stdio.h>
#include <math.h>
#include <string.h>
#include "kTree.h"



int main(int argc, char* argv[]){
	FILE *f;
	uint nodes; 
	ulong edges;
	register ulong i;
	
	if(argc<3){
		fprintf(stderr,"USAGE: %s <GRAPH> <outfile>\n",argv[0]);
		return(-1);
	}

	
	f = fopen(argv[1],"r");
  fread(&nodes,sizeof(uint),1,f);

  printf("> #nodes:\t%d\n", nodes);
	

  uint max_level = floor(log(nodes)/log(K)); 
  if(floor(log(nodes)/log(K))==(log(nodes)/log(K))) {
  	max_level=max_level-1;
  }
  fread(&edges,sizeof(ulong),1,f);
	uint nodes_read=0;

	printf("> #edges:\t%ld\n", edges);

	
	
	uint *xedges = (uint *)malloc(sizeof(uint)*edges);
	uint *yedges = (uint *)malloc(sizeof(uint)*edges);
	uint cedg = 0;
  for(i=0;i<nodes+edges;i++) {
    int k;
    fread(&k,sizeof(uint),1,f);
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

/*
 	rep->info = (uint *)malloc(sizeof(uint)*MAX_INFO);
	rep->element = (uint *)malloc(sizeof(uint)*MAX_INFO);	
	rep->basex = (uint *)malloc(sizeof(uint)*MAX_INFO);
	rep->basey = (uint *)malloc(sizeof(uint)*MAX_INFO);
	rep->iniq = -1;
	rep->finq =-1;
		rep->div_level_table = (uint *)malloc(sizeof(uint)*rep->maxLevel);
	for(i=0;i<rep->maxLevel;i++)
		rep->div_level_table[i]=exp_pow(K,rep->maxLevel-i);
*/

	saveRepresentation(rep,argv[2]);  
	destroyRepresentation(rep);
  
  free(xedges);
  free(yedges);
	fclose(f);
  return 0;
}


