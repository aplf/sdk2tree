#include <stdlib.h>
#include <stdio.h>

#define MAX 100000000

int main(int argc, char **argv) {
	if (argc != 3) {
		exit(-1);
	}
	FILE * f1 = fopen(argv[1], "r");
	FILE * f2 = fopen(argv[2], "r");

	int size1, size2;

	int * buf1 = (uint *) malloc (MAX * sizeof(uint));
	int * buf2 = (uint *) malloc (MAX * sizeof(uint));
	
	int elem1, elem2;
	int iter = 0;
	do {
		printf("iter %d\n", iter);
		size1 = fread(buf1, sizeof(uint), MAX, f1);
		size2 =	fread(buf2, sizeof(uint), MAX, f2);
		int i;
		for (i = 0; i < size1; i++) {
			elem1 = buf1[i];
			elem2 = buf2[i];
			if (elem1 < 0 && elem1 != elem2) printf("different - at iter %d pos %d\n", iter, i);
			if (elem1 >= 0 && elem1 != elem2-1) printf("different + at iter %d pos %d : %d VS %d\n", iter, i, elem1, elem2);
		}
		iter ++;	
	} while (size1 > 0);
	
	exit(0);
	


}


