#include <time.h>
#include "k2tree-common.h"
#include "timing.h"
#include <assert.h>

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
	
	K2Tree * k2tree;
//	int i;
	
	if (argc < 2) {
		printf("Usage: %s <file> <ff> <lf> <ft> <lt>", argv[0]);
		exit(EXIT_UNEXPECTED);
	}
	
	
	ticks= (double)sysconf(_SC_CLK_TCK);
	startTimer();

	k2tree = loadK2Tree(argv[1], 1);

	uint **v = findRange(k2tree, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));


//	uint * res = (uint *) malloc(v[0][0] * sizeof(uint));

//	for (i = 0; i < v[0][0]; i++) {
//		res[i] = i;
//	}

//	qsort(res, v[0][0], sizeof(uint), (__compar_fn_t)sorter);
//
//	printf("sorted!\n");
////	for (i = 0; i < v[0][0] - 1; i++) {
////		printf("%d->%d\n", v[0][res[i]+1], v[1][res[i]+1]);
////	}
//
//	FILE * fout = fopen("out","w+");
//	int val = 862664;
//	fwrite(&val, sizeof(uint), 1, fout);
//	fwrite(v[0], sizeof(uint), 1, fout);
//
//	val = 0;
////	fwrite(&val, sizeof(uint), 1, fout);
//	int lastFrom = 0;
//
//	for (i = 0; i < v[0][0]; i++) {
//		uint newFrom = v[0][res[i]+1] + 1;
//		uint newTo = v[1][res[i]+1] + 1;
////		printf("%d->%d\n", newFrom, newTo);
//		if (newFrom > lastFrom) {
//			uint j;
//			for (j = lastFrom + 1; j <= newFrom; j++) {
//				val--;
//				fwrite(&val, sizeof(uint), 1, fout);
//			}
//			lastFrom = newFrom;
//		}
////		fwrite(&newFrom, sizeof(uint), 1, fout);
//		fwrite(&newTo, sizeof(uint), 1, fout);
//	}
//	fclose(fout);
//
	printf("total:%d\n", v[0][0]);
//	for (i = 0; i < v[0][0]; i++) {
//		printf("%d->%d\n", v[0][1+i], v[1][1+i]);
//	}

	destroyK2Tree(k2tree);

	exit(0);
}
