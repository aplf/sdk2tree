#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>


#include "treeBlock.c"

#define cpuTime() clock()


trieNode *initializeTree(char **argv) {
	treeBlock B;
	treeNode node;
	uint16_t curFlag = 0;
	uint16_t subTreeSize;
	uint16_t level = 0;
	trieNode *t = (trieNode *) malloc(sizeof(trieNode));
	t->children[0] = t->children[1] = t->children[2] = t->children[3] = NULL;
	t->block = NULL;

	S1 = atoi(argv[1]);
	S2 = atoi(argv[2]);
	S3 = atoi(argv[3]);
	L1 = atoi(argv[4]);
	L2 = atoi(argv[5]);
	alpha = atof(argv[6]);
	nNodes = atoi(argv[7]);
	double d= nNodes + 0.0;
	strLen = log2(d)+0.999;

	printf("Got %d %d %d, %d %d, %lf, %d, %d\n", S1, S2, S3, L1, L2, alpha, nNodes, strLen);


	N1 = 4;

	// Tamaño maximo del bloque, probar con 96, 128, 256, 512, 1024
	Nt = S3;

	sizeArray = (uint16_t *) malloc(sizeof(uint16_t) * (Nt + 1));

	for (int i = 0; i <= Nt; ++i) {
		if (i > N1)
			N1 = 4 * (((uint16_t) ceil((double) N1 / alpha) + 3) / 4);
		sizeArray[i] = N1;
	}

	node.first = 0;
	node.second = 0;





	return t;
}

uint8_t str[50];

inline void getMorton(uint32_t x, uint32_t y, int nbits) {
	int i;
	for (int i = 0; i < nbits; i++) {
		int n= ((x >> (nbits-1-i)&1)<<1)|(y >> (nbits-1-i)&1);
		str[i]=n;
	}
//	for (int i = 0; i < nbits; i++) {
//		printf("%c", str[i]+'0');
//	}
//	printf("\n");
	str[nbits]=0;
}

int nfound = 0;

int main(int argc, char **argv)
 {

	int cmd;
    if (argc!= 8) {
    	printf("Usage: %s <S1> <S2> <S3> <L1> <L2> <alpha> <nodes>\n", argv[0]);
    	return EXIT_FAILURE;
    }


    uint64_t n, n1;
    n = nNodes;
    n1 = n;

    clock_t readTreeFromAdjStart = cpuTime();
    trieNode * t = initializeTree(argv);
    clock_t readTreeFromAdjStop = cpuTime();


    unsigned long addCtr = 0;
    unsigned long remCtr = 0;
    unsigned long listCtr = 0;
    unsigned long checkCtr = 0;


    int addsOngoing = 0;
    clock_t addsStart;
    clock_t addsStop;

    int remsOngoing = 0;
    clock_t remsStart;
    clock_t remsStop;

    int listsOngoing = 0;
    clock_t listsStart;
    clock_t listsStop;

    int checksOngoing = 0;
    clock_t checksStart;
    clock_t checksStop;


    long double total_save_time = +0.0000000f;

    /* Let us process instructions... */
    clock_t beginning = cpuTime();
    while (1) {
      /* printf("[%f] > ", cpuTime()); */
      cmd = getchar();
      if (cmd == EOF || cmd == '\n') {
        continue; /* Remove '\n'... */
      }

      uint32_t x, y, i;
      char buffer[1024];
//      printf("%c", cmd);
      switch(cmd) {
        case 'a':
          if( ! addsOngoing ) {
            addsStart = cpuTime();
            addsOngoing = 1;
          }
          scanf("%u%u", &x, &y);
          getMorton(x, y, strLen);
//          printf(" %c %d %d\n", cmd, x, y);
          insertTrie(t, str, strLen, strLen-1);

          addCtr++;
          break;
//        case 'd':
//          if( ! remsOngoing ) {
//            remsStart = cpuTime();
//            remsOngoing = 1;
//          }
//          scanf("%u%u", &x, &y);
//          getMorton(x, y, strLen);
//
//          deleteTrie(t, str, strLen, strLen-1);
//          remCtr++;
//          break;
        case 'l':
        {
          if( ! checksOngoing ) {
            checksStart = cpuTime();
            checksOngoing = 1;
          }
          scanf("%u%u", &x, &y);
          getMorton(x,y, strLen);
          printf("%d\n", isEdgeTrie(t, str, strLen, strLen-1));

          checkCtr++;
          break;
        }
//        case 'n':
//          if( ! listsOngoing ) {
//            listsStart = cpuTime();
//            listsOngoing = 1;
//          }
//          scanf("%d", &x);
//          uint * neighbors = getNeighbors(k2tree, x, -1);
//
//
//  	      int nn = neighbors[0];
//
//          printf("N[%u] ->", x);
//
//          /*for (i = 0; i < nn; i++) {
//            printf(" %u", neighbors[1+i]);
//          }*/
//          printf("%d\n", nn);
//          listCtr++;
//          break;
        case 'i':
        {
        	uint64_t treeSize = sizeTrie(t);//B.size();
			printf("Total size: %lu bytes\n", treeSize);
//			printf("Bits per edge of the graph: %f\n", ((float)treeSize*8)/nEdges);
			printf("Numero de bloques en la estructura: %lu\n", totalBlocks);
          break;
        }
        case 's':
        {
          if (listsOngoing) {
            listsStop = cpuTime();
            listsOngoing = 0;
          }
          if (remsOngoing) {
            remsStop = cpuTime();
            remsOngoing = 0;
          }
          if (addsOngoing) {
            addsStop = cpuTime();
            addsOngoing = 0;
          }
          if (checksOngoing) {
            checksStop = cpuTime();
            checksOngoing = 0;
          }


          scanf("%s", buffer);
          int l = strlen(buffer);
          printf("> Storing %s (%d bytes).\n", buffer, l);

          clock_t save_time_start = cpuTime();
          saveTrie(t, buffer);
          clock_t save_time_end = cpuTime();

          total_save_time = (long double) (save_time_end - save_time_start);

          //total_save_time = ((float)(save_time_end - save_time_start))/CLOCKS_PER_SEC;
          printf("Save time: %Lf\n", total_save_time);

          break;
        }
        case 'x':
        {
        	uint64_t treeSize = sizeTrie(t);//B.size();
			fprintf(stderr, "Total size: %lu bytes\n", treeSize);
//			printf("Bits per edge of the graph: %f\n", ((float)treeSize*8)/nEdges);
			fprintf(stderr, "Numero de bloques en la estructura: %lu\n", totalBlocks);
        }
        case 'z':
        {
          printf("> Exiting.\n");
          clock_t ending = cpuTime();
          printf("Loop time: %Lf\n", (long double)(ending - beginning));

          FILE *f = fopen("dynamic_trie_times.tsv", "a");

          fprintf(f, "start_time;loop_time;exclusive_save_time;time_per_add_op;time_per_rem_op;time_per_list_op;time_per_check_op;add_op_count;add_op_exclusive_time;rem_op_count;rem_op_exclusive_time;list_op_count;list_op_exclusive_time;check_op_count;check_op_exclusive_time\n");
          long double initTime = ((long double)(readTreeFromAdjStop - readTreeFromAdjStart))/CLOCKS_PER_SEC;
          long double loopTime = ((long double)(ending - beginning))/CLOCKS_PER_SEC;
          long double exclusiveAddTime = ((long double)(addsStop - addsStart))/CLOCKS_PER_SEC;
          long double exclusiveRemTime = ((long double)(remsStop - remsStart))/CLOCKS_PER_SEC;
          long double exclusiveListsTime = ((long double)(listsStop - listsStart))/CLOCKS_PER_SEC;
          long double exclusiveChecksTime = ((long double)(checksStop - checksStart))/CLOCKS_PER_SEC;
          fprintf(f, "%Lf;%Lf;%Lf;%Lf;%Lf;%Lf;%Lf;%lu;%Lf;%lu;%Lf;%lu;%Lf;%lu;%Lf\n",
              initTime,
              loopTime,
              ((long double)total_save_time)/CLOCKS_PER_SEC,
              addCtr == 0 ? -1 : loopTime / addCtr,
              remCtr == 0 ? -1 : loopTime / remCtr,
              listCtr == 0 ? -1 : loopTime / listCtr,
              checkCtr == 0 ? -1 : loopTime / checkCtr,
              addCtr,
              addCtr == 0 ? -1 : exclusiveAddTime,
              remCtr,
              remCtr == 0 ? -1 : exclusiveRemTime,
              listCtr,
              listCtr == 0 ? -1 : exclusiveListsTime,
              checkCtr,
              checkCtr == 0 ? -1 : exclusiveChecksTime
              );

          fclose(f);


          exit(EXIT_SUCCESS);
          break;
        }
        default:
          fprintf(stderr, "Invalid instruction: %c\n", cmd);
      }
      getchar(); /* Remove '\n'... */
    }

    return EXIT_SUCCESS;


 }



