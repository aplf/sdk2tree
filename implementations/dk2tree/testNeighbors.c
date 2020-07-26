#include <time.h>
#include "k2tree-common.h"
#include "timing.h"

void swap(int * array, int p1, int p2) {
	int aux = array[p1];
	array[p1] = array[p2];
	array[p2] = aux;
}

int * permute(int n) {
	int * ret = (int *) malloc(n*sizeof(int));
	int i;
	srand(7);
	for (i = 0; i < n; i++) {
		ret[i] = i;
	}
	for (i = 0; i < n; i++) {
		int newpos = rand() % (i+1);
		swap(ret, i, newpos);
	}
	return ret;
}


int main(int argc, char ** argv) {
	
	char * fileName;
	K2Tree * tree;
	int i;
	ull nNeighbors;
//	int from, lastFrom;
	
	if (argc < 2) {
		printf("Usage: %s <file>", argv[0]);
		exit(EXIT_UNEXPECTED);
	}
	
	fileName = argv[1];
	
	ticks= (double)sysconf(_SC_CLK_TCK);
	startTimer();

	tree = loadK2Tree(fileName, 1);
	
	DEBUG("time = %f\n", timeFromBegin());

	int n = tree->nNodesReal;
	int * perm = permute(n);
//
	DEBUG("time = %f\n", timeFromBegin());

	nNeighbors = 0;
	//int j;
	uint * nn;
	for (i = 0; i < tree->nNodesReal ; i++) {
		if (!(i%100000)) printf(" iter %d\n", i);
		nn = findNeighbors(tree, perm[i]);
		nNeighbors += nn[0];
//		for (j = 0; j < nn[0]; j++) {
//			printf("%d ", nn[1+j]);
//		}
//		printf("\n");
//		nNeighbors += findRevNeighbors(tree, i)[0];
	}

	free(perm);

	DEBUG("time = %lf, neighbors = %lu\n", timeFromBegin(), nNeighbors);

	destroyK2Tree(tree);

	exit(0);
}
