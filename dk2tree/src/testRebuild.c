#include "k2tree-common.h"
#include "stdlib.h"
#include "timing.h"

uint **v;

int sorter(void * p1, void * p2) {
	uint i1 = *((uint *)p1);
	uint i2 = *((uint *)p2);
	int res = v[0][i1+1] - v[0][i2+1];
	if (!res)
		res = v[1][i1+1] - v[1][i2+1];
	return res;
}


int main(int argc, char ** argv) {
	
	char * fileName;
	K2Tree * tree;

	if (argc < 3) {
		printf("Usage: %s <file> <outfile>", argv[0]);
		exit(EXIT_UNEXPECTED);
	}
	
	fileName = argv[1];

	tree = loadK2Tree(fileName, 0);

	ticks= (double)sysconf(_SC_CLK_TCK);
	startTimer();
	{
		uint i,j;
		FILE * fout = fopen(argv[2], "w+");
		uint curNode = 0;
		uint nNodes = (uint) tree->nNodesReal;
		uint nEdges = (uint) tree->nEdges;
		ull nNeighbors = 0;
		uint * neighbors;
		fwrite(&nNodes, sizeof(uint), 1, fout);
		fwrite(&nEdges, sizeof(uint), 1, fout);

		DEBUG("k2tree has %d nodes %d edges\n", nNodes, nEdges);

//		printNode(tree, tree->root1);
//		printNode(tree, tree->root2);

		if (format == EDGE_LIST) {
			//Rebuild using range and sorting should be faster
			if (tree->nEdges > DEF_BIG_SIZE) {
				ERROR("Edges will not fit in range search. Returning");
				exit(EXIT_UNEXPECTED);
			}

			v = findRange(tree, 0, tree->nNodesReal-1, 0, tree->nNodesReal-1);

			if (v[0][0] != tree->nEdges) {
				ERROR("Unexpected result. Got %d edges in full range (graph has %ld edges)\n", v[0][0], tree->nEdges);
				exit(EXIT_UNEXPECTED);
			}

			uint * res = (uint *) malloc(v[0][0] * sizeof(uint));

			for (i = 0; i < v[0][0]; i++) {
				res[i] = i;
			}

			qsort(res, v[0][0], sizeof(uint), (__compar_fn_t)sorter);


			for (j = 0; j < v[0][0]; j++) {
				uint from = v[0][res[j]+1];
				uint to = v[1][res[j]+1];
				fwrite(&from, sizeof(uint), 1, fout);
				fwrite(&to, sizeof(uint), 1, fout);
			}



		} else {

			for (i = 0; i < tree->nNodesReal; i++) {
				int foo;
				if(!(i%100000)) DEBUG("Finding neighbors of %d\n", i);
				curNode--;
				fwrite(&curNode, sizeof(int), 1, fout);
				neighbors = findNeighbors(tree, i);
				nNeighbors+=neighbors[0];

				for (j = 0; j < neighbors[0]; j++) {
					foo = (int) neighbors[j+1]+1;
					fwrite(&foo, sizeof(int), 1, fout);
				}
			}
		}
		destroyK2Tree(tree);

		INFO("time = %f, neighbors = %lu, us/e = %lf\n", timeFromBegin(), nNeighbors, timeFromBegin()*1000000.0/nNeighbors);
	}
	exit(0);
}
