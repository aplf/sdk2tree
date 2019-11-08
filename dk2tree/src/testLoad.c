#include "k2tree-common.h"
#include "timing.h"


int main(int argc, char ** argv) {
	

	char * fileName;
	
	if (argc < 6) {
		printf("Usage: %s <file> <ffrom> <lfrom> <fto> <lto>", argv[0]);
		exit(EXIT_UNEXPECTED);
	}
	
	
	fileName = argv[1];
	
	{
		K2Tree * tree = loadK2Tree(fileName, 1);

		int fFrom = atoi(argv[2]);
		int lFrom = atoi(argv[3]);
		int fTo = atoi(argv[4]);
		int lTo = atoi(argv[5]);
		uint from = 0, to = 5;
	
		ticks= (double)sysconf(_SC_CLK_TCK);
		startTimer();
	
	


		for (from = fFrom; from < lFrom ; from ++){
			for (to = fTo; to < lTo; to++){
				byte res = findEdge(tree, from , to);
				if (res) DEBUG("%4d->%4d\n", from, to);
			}
		}

		destroyK2Tree(tree);

		DEBUG("time = %f\n", timeFromBegin());
	}
	exit(0);
}
