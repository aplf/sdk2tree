#include "k2tree-common.h"
#include "timing.h"

int main(int argc, char ** argv) {
	
	if (argc < 6) {
		printf("Usage: %s <sourceFile> <graphFile> <blockSize> <njumps> (<kValue> <nLevels>)* <lastKValue> [<leafLevels>]\n", argv[0]);
		exit(EXIT_UNEXPECTED);
	}
	{
		char * sourceFile = argv[1];
		char * graphFile = argv[2];
		char * vocFile = (char *)malloc(256*sizeof(char));
		uint blockSize = atoi(argv[3]);
		uint njumps = atoi(argv[4]);
		K2Tree *tree;
		uint i;

		uint nvalues = argc - argc%2 - 5;
		uint leafLevels = (argc % 2)? atoi(argv[argc-1]) : 1;
		uint * values;

		strcpy(vocFile, "dic-wg-8.voc"); //FIXME: Variable, pasar como parametro se se quere usar
//		strcpy(vocFile, "onlyeu-2005.voc");

		values = (uint *) malloc(nvalues * sizeof(uint));
		for (i = 0; i < nvalues; i++) {
			values[i] = atoi(argv[5+i]);
		}


		ticks= (double)sysconf(_SC_CLK_TCK);
		startTimer();
	
		tree = createK2Tree(sourceFile, graphFile, vocFile, blockSize, njumps, nvalues, values, leafLevels);

//		DEBUG("Delete node\n");
//		deleteNode(tree, 0);

//		DEBUG("Get neighbors\n");
//		uint *n1 = findNeighbors(tree, 0);
//		if (n1[0]) DEBUG("Has %d neighbors\n", n1[0]);
//		n1 = findRevNeighbors(tree, 0);
//		if (n1[0]) DEBUG("Has %d revneighbors\n", n1[0]);

//
//		DEBUG("k2tree has %d nodes\n", tree->nNodesReal);
//
//
//		INFO("Creation : %f\n", timeFromBegin());

//		rebuildLastLevel(tree);

//		replaceByOptimal(tree->voc);

//		if (tree->useDictionary) {
//			INFO("%d\n", getCompressedSize(tree->voc));
//			INFO("%d\n", getOptimalSize(tree->voc));
//			INFO("%d\n", getFixedSize(tree->voc));
//		}

		free(values);
		free(vocFile);

		INFO("Memory usage : %ld\n", memUsageK2Tree(tree));
		INFO("Disk usage : %ld\n", diskUsageK2Tree(tree));

		destroyK2Tree(tree);

		INFO("Creation time = %f\n", timeFromBegin());
	}
	exit(0);
}
