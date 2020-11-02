#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>


#include "kTree.h"
#include "adylist.h"

#include <assert.h>

#define cpuTime() clock()


int
main(int argc, char *argv[]) {

  int cmd;

  // Example program call:
  // ./others/k2tree/project prepared_datasets/dmgen/100000/100000 100000_lists_eval time_results_out_file < <( eval/streamer.sh prepared_datasets/dmgen/100000/100000-lists.tsv ) > /dev/null 2>&1
  /* if(argc<4){
		fprintf(stderr,"USAGE: %s <GRAPH> <outfile> <time_outfile>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
  */
  if(argc<3){
		fprintf(stderr,"USAGE: %s <GRAPH> <outfile>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

  // Initialize the static k2tree which is created from a binary adjacency file.
  clock_t readTreeFromAdjStart = cpuTime();
  MREP * rep = loadRepresentation(argv[1]);
  clock_t readTreeFromAdjStop = cpuTime();


  unsigned long listCtr = 0;
  unsigned long checkCtr = 0;

  int listsOngoing = 0;
  clock_t listsStart;
  clock_t listsStop;

  int checksOngoing = 0;
  clock_t checksStart;
  clock_t checksStop;

  /* Let us process instructions... */
  clock_t beginning = cpuTime();
  while (1) {
    /* printf("[%f] > ", cpuTime()); */
    cmd = getchar();

    if (cmd == EOF || cmd == '\n') {
      continue; /* Remove '\n'... */
    }

    uint32_t x, y;//, i;
    
    char buffer[1024];
    

    switch(cmd) {
      case 'l': // check edge existence
        if( ! checksOngoing ) {
          checksStart = cpuTime();
          checksOngoing = 1;
        }
        scanf("%u%u", &x, &y);
      	uint resp = compact2CheckLinkQuery(rep, x, y);
        printf("%u\n", resp);

        checkCtr++;
        break;
      case 'n': // get neighbors
        if( ! listsOngoing ) {
          listsStart = cpuTime();
          listsOngoing = 1;
        }
        scanf("%d", &x);
        uint * listady = compact2AdjacencyList(rep, x);
        printf("(%u)\t", listady[0]);
        /*for(i=0; i<listady[0]; i++){
          printf("%u\t", listady[i+1]);
        }*/
        printf("\n");
        listCtr++;
        break;
      case 's':
        if (listsOngoing) {
          listsStop = cpuTime();
          listsOngoing = 0;
        }
        if (checksOngoing) {
          checksStop = cpuTime();
          checksOngoing = 0;
        }


        scanf("%s", buffer);
        
        //strlen(buffer);
        //int l = strlen(buffer);


        break;
      case 'x':
       	destroyRepresentation(rep);
      case 'z':
        printf("> Exiting.\n");
        clock_t ending = cpuTime();
        printf("Loop time: %Lf\n", (long double)(ending - beginning));

        //char resname[256];
        //memset(resname, '\0', 256*sizeof(char));
        //strcpy(resname, argv[3]);
        //strcat(resname, "_static_k2tree_times.tsv");
        //FILE *f = fopen(resname, "w");
        FILE *f = fopen("k2tree_times.tsv", "a");
       
        fprintf(f, "start_time;loop_time;time_per_list_op;time_per_check_op;list_op_count;list_op_exclusive_time;check_op_count;check_op_exclusive_time\n");
        long double initTime = ((long double)(readTreeFromAdjStop - readTreeFromAdjStart))/CLOCKS_PER_SEC;
        long double loopTime = ((long double)(ending - beginning))/CLOCKS_PER_SEC;
        long double exclusiveListsTime = ((long double)(listsStop - listsStart))/CLOCKS_PER_SEC;
        long double exclusiveChecksTime = ((long double)(checksStop - checksStart))/CLOCKS_PER_SEC;
        fprintf(f, "%Lf;%Lf;%Lf;%Lf;%lu;%Lf;%lu;%Lf\n", 
            initTime,
            loopTime,
            listCtr == 0 ? -1 : loopTime / listCtr,
            checkCtr == 0 ? -1 : loopTime / checkCtr,
            listCtr,
            listCtr == 0 ? -1 : exclusiveListsTime,
            checkCtr,
            checkCtr == 0 ? -1 : exclusiveChecksTime
            );
       
        

        /* 
        fprintf(f, "start_time;loop_time;time_per_list_op\n");
        long double initTime = (long double)(readTreeFromAdjStop - readTreeFromAdjStart);
        long double loopTime = (long double)(ending - beginning);
        fprintf(f, "%Lf;%Lf;%Lf\n", initTime, loopTime, opCtr == 0 ? -1 : loopTime / opCtr);
        */



        fclose(f);

        exit(EXIT_SUCCESS);
        break;
      default:
        fprintf(stderr, "Invalid instruction: %c\n", cmd);
    }
    getchar(); /* Remove '\n'... */
  }

  return EXIT_SUCCESS;
}
