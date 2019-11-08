#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include <assert.h>

#include "k2tree-common.h"
#include "timing.h"

#define cpuTime() (clock()*1e-6)

void testInitialK2Tree(K2Tree * k2tree) {
  // Testing some insertions.
  for(int i = 0; i < 99; i++) {
    insertEdge(k2tree, i, i+1);
    printf("Inserted (%d->%d)\n", i, i+1);
  }

  //
  for(int i = 0; i < 99; i++) {
    insertEdge(k2tree, i, i+1);
    printf("Inserted (%d->%d)\n", i, i+1);
  }
}

K2Tree * init_structure(int argc, char * argv[]) {
  printf("> Running dynamic k2tree (UDC). \n");

  // Step 1: parse parameters as in the main() of testCreateFromGraph.c.
  // Old parameters received in argv[] of testCreateFromGraph.c or defined in it.
  // Example program call of the 'graph' program:
  // ./src/graph cnr-2000 generated 512 4 4 3 2
  // NOTE: cnr-2000 will have been converted from the WebGraph format using Transforma.java

  //char * sourceFileName = argv[1];
  char * outGraphFile = argv[1];
  char * vocFile = (char *)malloc(256*sizeof(char));
  uint nodes, nodesOrig;
  nodes = atoi(argv[2]);
  nodesOrig = 0;

  uint blockSize = atoi(argv[3]);
  uint njumps = atoi(argv[4]);
  //uint i;

  uint nvalues = argc - argc%2 - 5;
  uint leafLevels = (argc % 2)? atoi(argv[argc-1]) : 1;
  uint * values;


  strcpy(vocFile, "dic-wg-8.voc"); //FIXME: Variable, pasar como parametro se se quere usar

  values = (uint *) malloc(nvalues * sizeof(uint));
  for (uint i = 0; i < nvalues; i++) {
    values[i] = atoi(argv[5+i]);
  }
  
  printf("> Finished parsing arguments. \n");

  // Step 2: create empty K2Tree as in createK2Tree (defined in k2tree-common.c, used in testCreateFromGraph.c).
  // Instead of calling createK2Tree here as in testCreateFromGraph.c, we execute its internals.
  // tree = createK2Tree(sourceFile, outGraphFile, vocFile, blockSize, njumps, nvalues, values, leafLevels);
	uint nLevels;
  uint * kValues = getKForLevels(nvalues, values, &nodes, &nLevels, leafLevels);

  printf("> Got kValues.\n");

	K2Tree * k2tree = createEmptyTree(outGraphFile, nodes, nodesOrig, blockSize, njumps, nLevels, kValues);

  printf("> Created empty K2Tree.\n");


	initStack(&k2tree->emptyNodes, DEF_SMALL_SIZE);

  printf("> K2Tree stack initiated.\n");

	if (k2tree->useDictionary) {
		int wl = k2tree->kValues[k2tree->nLevels-1]*k2tree->kValues[k2tree->nLevels-1]/8;
		k2tree->voc = createEmptyVocabulary(wl);
	}


  
  

  return k2tree;
}



int
main(int argc, char *argv[]) {

  int cmd;

  // Example program call:
  // ./src/project myGraph 100 512 4 4 3 2
  // ./project myGraph 100 512 4 4 3 2 < ../../../datasets/additions-test.tsv
  if (argc < 6) {
    printf("Usage: %s <outGraphFile> <num_nodes> <blockSize> <njumps> (<kValue> <nLevels>)* <lastKValue> [<leafLevels>]\n", argv[0]);
    return EXIT_FAILURE;
  }

  clock_t readTreeFromAdjStart = cpuTime();
  K2Tree * k2tree = init_structure(argc, argv);
  clock_t readTreeFromAdjStop = cpuTime();

  //testInitialK2Tree(k2tree);

  printf("> Initial K2Tree info:\n");
  //displayTreeInfo(k2tree);

  //return 0;

  unsigned long addCtr = 0;
  unsigned long remCtr = 0;
  unsigned long listCtr = 0;

  
  int addsOngoing = 0;
  clock_t addsStart;
  clock_t addsStop;

  int remsOngoing = 0;
  clock_t remsStart;
  clock_t remsStop;

  int listsOngoing = 0;
  clock_t listsStart;
  clock_t listsStop;


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
    //sleep(1);
    int prev, post;
    switch(cmd) {
      case 'a':
        if( ! addsOngoing ) {
          addsStart = cpuTime();
          addsOngoing = 1;
        }
        scanf("%u%u", &x, &y);
        insertEdge(k2tree, x, y);
        addCtr++;
//        prev = findEdge(k2tree, x, y);
//        insertEdge(k2tree, x, y);
//        post = findEdge(k2tree, x, y);
//        ASSERT(!prev && post);
        break;
      case 'd':
        if( ! remsOngoing ) {
          remsStart = cpuTime();
          remsOngoing = 1;
        }
        scanf("%u%u", &x, &y);
//        prev = findEdge(k2tree, x, y);
        //prev = findEdge(k2tree, x, y);
        removeEdge(k2tree, x, y);
        //post = findEdge(k2tree, x, y);
        //ASSERT(prev & !post);
        remCtr++;
        break;
      case 'l':
        scanf("%u%u", &x, &y);
        printf("%d\n", findEdge(k2tree, x, y));
        break;
      case 'n':
        if( ! listsOngoing ) {
          listsStart = cpuTime();
          listsOngoing = 1;
        }
        scanf("%d", &x);
        uint * neighbors = getNeighbors(k2tree, x, -1);


	      int nn = neighbors[0];

        printf("N[%u] ->", x);

        for (i = 0; i < nn; i++) {
          printf(" %u", neighbors[1+i]);
        }
        printf("\n");
        listCtr++;

        break;
      case 'i':
      /*
        printf("#V:%d #E: %d\n", rep.nv, rep.ne);
        printf("r = %d\n", rep.r);
        for (i = 0; i <= rep.maxr+1; i++)
          printf("max_%d = %f\n", i, MAXSZ(max(rep.nv,rep.ne), i));
        printf("htsz = %d\n", rep.htsz);
        printf("elsz = %d\n", rep.elsz);
        printf("adjsz = %d\n", rep.adjsz);
        printf("C[0] = %d\n", rep.eln);
        for (i = 0; i <= rep.maxr; i++)
          if (rep.k2t[i] != NULL)
            printf("C[%d] = %lu\n", i+1, rep.k2t[i]->numberOfEdges);

            */

        displayTreeInfo(k2tree);

        break;
      case 's':

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

        scanf("%s", buffer);
        int l = strlen(buffer);
        printf("> Storing %s (%d bytes).\n", buffer, l);

        // TODO: write representation to disk

        /*
        for (i = 0; i <= rep.maxr; i++)
          if (rep.k2t[i] != NULL) {
            sprintf(buffer+l, ".%d", i+1);
            saveRepresentation(rep.k2t[i], buffer);
          }

          */

        clock_t save_time_start = cpuTime();
        /*
        for (i = 0; i <= rep.maxr; i++) {
          if (rep.k2t[i] != NULL) {
            sprintf(buffer+l, ".%d", i+1);
            saveRepresentation(rep.k2t[i], buffer);
          }
        }
        */
        clock_t save_time_end = cpuTime();

        total_save_time = (long double) (save_time_end - save_time_start);

        //total_save_time = ((float)(save_time_end - save_time_start))/CLOCKS_PER_SEC;
        printf("Save time: %Lf\n", total_save_time);


        break;
      case 'x':

       	printf("Memory usage : %ld\n", memUsageK2Tree(k2tree));
        printf("Disk usage : %ld\n", diskUsageK2Tree(k2tree));

        destroyK2Tree(k2tree);

        printf("Creation time = %f\n", timeFromBegin());
        
        break;
      case 'z':
        printf("> Exiting.\n");
        clock_t ending = cpuTime();
        printf("Loop time: %Lf\n", (long double)(ending - beginning));

        FILE *f = fopen("dynamic_UPC_k2tree_times.tsv", "w");
        
        fprintf(f, "start_time;loop_time;exclusive_save_time;time_per_add_op;time_per_rem_op;time_per_list_op;add_op_count;add_op_exclusive_time;rem_op_count;rem_op_exclusive_time;list_op_count;list_op_exclusive_time\n");
        long double initTime = (long double)(readTreeFromAdjStop - readTreeFromAdjStart);
        long double loopTime = (long double)(ending - beginning);
        long double exclusiveAddTime = (long double)(addsStop - addsStart);
        long double exclusiveRemTime = (long double)(remsStop - remsStart);
        long double exclusiveListsTime = (long double)(listsStop - listsStart);
        fprintf(f, "%Lf;%Lf;%Lf;%Lf;%Lf;%Lf;%lu;%Lf;%lu;%Lf;%lu;%Lf\n", 
            initTime,
            loopTime,
            total_save_time,
            addCtr == 0 ? -1 : loopTime / addCtr,
            remCtr == 0 ? -1 : loopTime / remCtr,
            listCtr == 0 ? -1 : loopTime / listCtr,
            addCtr,
            addCtr == 0 ? -1 : exclusiveAddTime,
            remCtr,
            remCtr == 0 ? -1 : exclusiveRemTime,
            listCtr,
            listCtr == 0 ? -1 : exclusiveListsTime
            );

        fclose(f);
        exit(EXIT_SUCCESS);
        break;
      default:
        fprintf(stderr, "Invalid instruction: %c\n", cmd);
        printf("last = %d %d\n", x, y);
        exit(3);
    }
    getchar(); /* Remove '\n'... */
//    printf("%d %d %d\n", cmd, x, y);
  }

  return EXIT_SUCCESS;
}
