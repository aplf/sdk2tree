#include "k2tree-common.h"
#include "stdlib.h"
#include "timing.h"

int main(int argc, char ** argv) {
	
	char * fileName;
	K2Tree * tree;

	if (argc < 3) {
		printf("Usage: %s <file> <outfile>", argv[0]);
		exit(EXIT_UNEXPECTED);
	}
	
	fileName = argv[1];

	tree = loadK2Tree(fileName, 0);

	rebuildLastLevel(tree);

	replaceByOptimal(tree->voc);

	destroyK2Tree(tree);

	exit(0);
}
