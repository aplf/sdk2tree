#include "k2tree-common.h"

int main(int argc, char ** argv) {
	
	char * fileName;
	
	if (argc < 2) {
		printf("Usage: %s <file>", argv[0]);
		exit(EXIT_UNEXPECTED);
	}
	
	fileName = argv[1];
	
	{
		K2Tree * tree = loadK2Tree(fileName, 0);

		int from = atoi(argv[2]);
		int to = atoi(argv[3]);
		byte res;

		insertEdge(tree, from , to);
		DEBUG("Inserted (%d->%d)\n", from, to);

		res = findEdge(tree, from, to);
		if (!res)
			ERROR("Failed! Node %d->%d not inserted properly\n", from, to);
		DEBUG("Result of getting value = %d\n", res);


		DEBUG("Destroying k2tree\n");
		destroyK2Tree(tree);
	}
	exit(0);
}
