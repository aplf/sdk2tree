#include "k2tree-common.h"
#include "timing.h"

int main(int argc, char ** argv) {
	
	int foo;
	if (argc != 3) {
		printf("Usage: %s <sourceFile> <graphFile>\n", argv[0]);
		exit(EXIT_UNEXPECTED);
	}
	
	
	ticks= (double)sysconf(_SC_CLK_TCK);
	startTimer();
	
	{
		char * sourceFileName = argv[1];
		char * graphFile = argv[2];

		int i, val;
		uint nodesOrig, edges;
		int currentNode;
		int res;
		FILE * sourceFile = fopen(sourceFileName,"r");
	
		K2Tree * k2tree = loadK2Tree(graphFile, 1);

	
		sourceFile = fopen(sourceFileName,"r");
		foo = fread(&(nodesOrig),sizeof(uint),1,sourceFile);
		if (foo != 1)
			exit(IO_ERROR);
		foo = fread(&(edges),sizeof(uint),1,sourceFile);


		currentNode = -1;
		for  (i  = 0; i < nodesOrig + edges; i++) {
			foo = fread(&(val), sizeof(uint), 1, sourceFile);
			if (foo != 1){
				exit(IO_ERROR);
			}
			if (val < 0) {
				currentNode++;
				if (!(currentNode % 1000)) DEBUG("Current node = %d\n", currentNode);
			} else {
				res = findEdge(k2tree, currentNode, val-1);
				if (!res) {
					ERROR("Failed for node %d->%d\n", currentNode, val-1);
				}
			}
		}
	
	
		fclose(sourceFile);


		destroyK2Tree(k2tree);

		DEBUG("time = %f\n", timeFromBegin());
	}
	exit(0);
}
