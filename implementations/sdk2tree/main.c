#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include <assert.h>

#include <stdbool.h>

#include "kTree.h"
uint exp_pow(uint base, uint pow);

#include "kt_setOperations.h"

#include "bitrankw32int.h"

#define cpuTime() clock()

struct adje {
  uint32_t x, nxt;
};

struct edge {
  uint32_t x, y, nxt, prv;
};

struct data {
  uint32_t *htable;    /* Hash table to track edges. */
  struct adje * adj;   /* Adjacency hash table. */
  struct edge * elst;  /* Edge array. */
  uint32_t *efree;     /* Edge array empty positions. */
  uint32_t htsz;       /* Memory allocated for 'htable'. */
  uint32_t htn;        /* Number of elements in 'htable'. */
  uint32_t adjsz;      /* Memory allocated for 'adj'. */
  uint32_t adjn;       /* Number of elements in 'adj'. */
  uint32_t elsz;       /* Memory allocated for 'elst'. */
  uint32_t eln;        /* Number of elements in 'elst'. */
  uint32_t nv;         /* Number of vertices. */
  uint32_t ne;         /* Number of edges. */
  /* k2tree stuff */
  uint32_t r;          /* Memory allocated for 'k2t'. */
  uint32_t maxr;       /* Max index to check in 'k2t'. */
  uint32_t k;
  MREP ** k2t;         /* Collection of k2trees. */
  /* k2trees shared aux data. */
  uint * div_level_table;
  uint * info;
  uint * info2[2];
  uint * element;
  uint * basex;
  uint * basey;
};

static uint32_t hash32shift(uint32_t key);
static uint32_t hash6432shift(uint64_t key);

static uint32_t * hinsert(uint32_t *ht, uint32_t *htsz, uint32_t *htn,
  struct edge *elst, uint32_t li);
static uint32_t hfind(uint32_t *ht, uint32_t *htsz, struct edge *elst,
  uint32_t x, uint32_t y);
static uint32_t *
hdelete(uint32_t *ht, uint32_t *htsz, uint32_t *htn, struct edge *elst,
  uint32_t li);

static struct adje * adjinsert(struct adje *adj, uint32_t *adjsz,
  uint32_t *adjn, uint32_t x, uint32_t nxt);
static uint32_t adjfind(struct adje *adj, uint32_t *adjsz, uint64_t x);

static void init_data(struct data *p, uint32_t nv);

static void add_link(struct data *p, uint32_t x, uint32_t y);
static void del_link(struct data *p, uint32_t x, uint32_t y);
static int check_link(struct data *p, uint32_t x, uint32_t y);
static void list_neighbors(struct data *p, uint32_t x);

#define EPS 0.25
#define TAU(m) log2(log2(m))
#define MAXSZ(m, i) floor(((m) << 1)/pow(log2(m),2 - (i)*EPS))

int
main(int argc, char *argv[]) {

  struct data rep;
  int cmd;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <number of vertices>\n", argv[0]);
    return EXIT_FAILURE;
  }

  printf(" Running dynamic k2tree (INESC-ID). \n");

  clock_t readTreeFromAdjStart = cpuTime();
  init_data(&rep, atoi(argv[1]));
  clock_t readTreeFromAdjStop = cpuTime();


  unsigned long addCtr = 0;
  unsigned long remCtr = 0;
  unsigned long listCtr = 0;
  unsigned long checkCtr = 0;

  bool addsOngoing = false;
  clock_t addsStart;
  clock_t addsStop;

  bool remsOngoing = false;
  clock_t remsStart;
  clock_t remsStop;

  bool listsOngoing = false;
  clock_t listsStart;
  clock_t listsStop;

  bool checksOngoing = false;
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

    //printf("%c\n", (char)cmd);

    //int save_ctr = 0;

    switch(cmd) {
      case 'a':
        if( ! addsOngoing ) {
          addsStart = cpuTime();
          addsOngoing = true;
        }
        scanf("%u%u", &x, &y);
        add_link(&rep, x, y);
        addCtr++;
        break;
      case 'd':
        if( ! remsOngoing ) {
          remsStart = cpuTime();
          remsOngoing = true;
        }
        scanf("%u%u", &x, &y);
        del_link(&rep, x, y);
        remCtr++;
        break;
      case 'l':
        if( ! checksOngoing ) {
          checksStart = cpuTime();
          checksOngoing = true;
        }
        scanf("%u%u", &x, &y);
        printf("%d\n", check_link(&rep, x, y));
        checkCtr++;
        break;
      case 'n':
        if( ! listsOngoing ) {
          listsStart = cpuTime();
          listsOngoing = true;
        }
        scanf("%d", &x);
        list_neighbors(&rep, x);
        listCtr++;
        break;
      case 'i':
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
        break;
      case 's':
        if (listsOngoing) {
          listsStop = cpuTime();
          listsOngoing = false;
        }
        if (remsOngoing) {
          remsStop = cpuTime();
          remsOngoing = false;
        }
        if (addsOngoing) {
          addsStop = cpuTime();
          addsOngoing = false;
        }
        if (checksOngoing) {
          checksStop = cpuTime();
          checksOngoing = false;
        }

        scanf("%s", buffer);
        int l = strlen(buffer);

        clock_t save_time_start = cpuTime();

        sprintf(buffer+l, ".0.kt");
        FILE* fout = fopen(buffer, "w");
        fwrite(&(rep.nv), sizeof(uint32_t), 1, fout);
        fwrite(&(rep.ne), sizeof(uint32_t), 1, fout);
        fwrite(&(rep.eln), sizeof(uint32_t), 1, fout);
        for (i = 0; i < rep.eln; ++i)
          fwrite(&(rep.elst[rep.efree[i]]), sizeof(uint32_t), 2, fout);
        fclose(fout);

        for (i = 0; i <= rep.maxr; i++) {
          if (rep.k2t[i] != NULL) {
            sprintf(buffer+l, ".%d", i+1);
            saveRepresentation(rep.k2t[i], buffer);
          }
        }
        clock_t save_time_end = cpuTime();

        total_save_time = (long double) (save_time_end - save_time_start);
        //total_save_time = ((float)(save_time_end - save_time_start))/CLOCKS_PER_SEC;

        printf("Save time: %Lf\n", total_save_time);
        break;
      case 'x':
        free(rep.htable);
        free(rep.adj);
        free(rep.elst);
        free(rep.efree);
        for (i = 0; i < rep.r; i++)
          if (rep.k2t[i] != NULL) {
            destroyBitRankW32Int(rep.k2t[i]->btl);
            free(rep.k2t[i]);
          }
        free(rep.k2t);
        free(rep.div_level_table);
        free(rep.info);
        free(rep.info2[0]);
        free(rep.info2[1]);
        free(rep.element);
        free(rep.basex);
        free(rep.basey);
      case 'z':
        printf("> Exiting.\n");
        clock_t ending = cpuTime();
        printf("Loop time: %f\n", ((float)(ending - beginning))/CLOCKS_PER_SEC);

        FILE *f = fopen("dynamic_INESC_k2tree_times.tsv", "a");


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
      default:
        fprintf(stderr, "Invalid instruction: %c\n", cmd);
    }

    getchar(); /* Remove '\n'... */
    

    
  }

  return EXIT_SUCCESS;
}

static uint32_t
hash32shift(uint32_t key) {

  key = ~key + (key << 15);
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = (key + (key << 3)) + (key << 11);
  key = key ^ (key >> 16);

  return key;
}

static uint32_t
hash6432shift(uint64_t key) {

  key = (~key) + (key << 18);
  key = key ^ (key >> 31);
  key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);

  return (uint32_t) key;
}

static uint32_t *
hinsert(uint32_t *ht, uint32_t *htsz, uint32_t *htn, struct edge *elst,
  uint32_t li) {

  uint64_t tmp;
  tmp = elst[li].x;
  tmp <<= 32;
  tmp |= elst[li].y;

  if (*htn > ((*htsz) >> 1)) {

    (*htsz) <<= 1;
    uint32_t *ht_old = ht;
    ht = malloc(sizeof(uint32_t) * (*htsz));
    memset(ht, 0xff, sizeof(uint32_t) * (*htsz));
    *htn = 0;

		uint32_t k;
    for (k = 0; k < ((*htsz) >> 1); k++)
      if (ht_old[k] != 0xffffffff)
        ht = hinsert(ht, htsz, htn, elst, ht_old[k]);

    free(ht_old);
  }

  uint32_t i = hash6432shift(tmp) % (*htsz);
  while (ht[i] != 0xffffffff)
    i = (i+1) % (*htsz);

  ht[i] = li;
  (*htn) ++;

  return ht;
}

static uint32_t
hfind(uint32_t *ht, uint32_t *htsz, struct edge * elst,
  uint32_t x, uint32_t y) {

  uint64_t tmp;
  tmp = x;
  tmp <<= 32;
  tmp |= y;
  uint32_t i = hash6432shift(tmp) % (*htsz);

  while (ht[i] != 0xffffffff)
    if (x == elst[ht[i]].x && y == elst[ht[i]].y)
      return ht[i];
    else
      i = (i+1) % (*htsz);

  return 0xffffffff;
}

static uint32_t *
hdelete(uint32_t *ht, uint32_t *htsz, uint32_t *htn, struct edge *elst,
  uint32_t li) {

  uint64_t tmp;
  tmp = elst[li].x;
  tmp <<= 32;
  tmp |= elst[li].y;
  uint32_t i = hash6432shift(tmp) % (*htsz);

  while (li != ht[i])
    i = (i+1) % (*htsz);

  ht[i] = 0xffffffff;
  i = (i+1) % (*htsz);

  while (ht[i] != 0xffffffff) {
    li = ht[i];
    ht[i] = 0xffffffff;
    ht = hinsert(ht, htsz, htn, elst, li);
    i = (i+1) % (*htsz);
  }

  (*htn) --;

  return ht;
}


static struct adje *
adjinsert(struct adje *adj, uint32_t *adjsz, uint32_t *adjn, uint32_t x,
  uint32_t nxt) {

  if (*adjn > ((*adjsz) >> 1)) {

    (*adjsz) <<= 1;
    struct adje *adj_old = adj;
    adj = malloc(sizeof(struct adje) * (*adjsz));
    memset(adj, 0xff, sizeof(struct adje) * (*adjsz));
    *adjn = 0;

		uint32_t k;
    for (k = 0; k < ((*adjsz) >> 1); k++)
      if (adj_old[k].x != 0xffffffff)
        adj = adjinsert(adj, adjsz, adjn, adj_old[k].x, adj_old[k].nxt);

    free(adj_old);
  }

  uint32_t i = hash32shift(x) % (*adjsz);
  while (adj[i].x != 0xffffffff)
    i = (i+1) % (*adjsz);

  adj[i].x = x;
  adj[i].nxt = nxt;
  (*adjn) ++;

  return adj;
}

static uint32_t
adjfind(struct adje *adj, uint32_t *adjsz, uint64_t x) {
  uint32_t i = hash32shift(x) % (*adjsz);

  while (adj[i].x != 0xffffffff)
    if (x == adj[i].x)
      return i;
    else
      i = (i+1) % (*adjsz);

  return 0xffffffff;
}

static void
init_data(struct data * p, uint32_t nv) {
  p->nv = nv;
  p->ne = 0;
  uint32_t max = MAXSZ(p->nv, 0);
  uint32_t i;

  p->elsz = max;
  p->elst = malloc(sizeof(struct edge)*p->elsz);
  memset(p->elst, 0x0, sizeof(struct edge)*p->elsz);
  p->efree = malloc(sizeof(uint32_t)*p->elsz);
  for (i = 0; i < p->elsz; i++)
    p->efree[i] = i;
  p->eln = 0;

  p->htsz = max << 1;
  p->htable = malloc(sizeof(uint32_t)*p->htsz);
  memset(p->htable, 0xff, sizeof(uint32_t)*p->htsz);
  p->htn = 0;

  /* The number of vertices in G_0 is also bound by max_0. */
  p->adjsz = max << 1;
  p->adj = malloc(sizeof(struct adje)*p->adjsz);
  memset(p->adj, 0xff, sizeof(struct adje)*p->adjsz);
  p->adjn = 0;

  p->k = 2;
  /* 'r' should be updated to 'ne' once 'ne > nv'. */
  /* p->r = 1 + 2*log2(log2(p->nv)); */
  p->r = 8; /* Limit as m->\infty; depends on EPS. */
  p->maxr = 0;
  p->k2t = malloc(sizeof(MREP *)*p->r);
  memset(p->k2t, 0x0, sizeof(MREP *)*p->r);

  uint32_t max_level = floor(log(p->nv)/log(p->k));
  if(floor(log(p->nv)/log(p->k)) == (log(p->nv)/log(p->k)))
    max_level = max_level-1;
  p->div_level_table = (uint *)malloc(sizeof(uint)*(max_level+1));
  for(i = 0; i <= max_level; i++)
    p->div_level_table[i] = exp_pow(K,max_level-i);
  p->info = (uint *)malloc(sizeof(uint)*(p->nv+10));
  p->element = NULL; // (uint *)malloc(sizeof(uint)*(p->nv+10));
  p->basex = NULL; // (uint *)malloc(sizeof(uint)*(p->nv+10));
  p->basey = NULL; // (uint *)malloc(sizeof(uint)*(p->nv+10));
  p->info2[0] = NULL; // (uint *)malloc(sizeof(uint)*(p->nv+10));
  p->info2[1] = NULL; // (uint *)malloc(sizeof(uint)*(p->nv+10));
}

static void
add_link_0(struct data *p, uint32_t x, uint32_t y) {
  uint32_t i, k;

  if (hfind(p->htable, &(p->htsz), p->elst, x, y) == 0xffffffff) {

    if (p->eln >= p->elsz) {
      uint32_t new_elsz = MAXSZ(max(p->nv,p->ne), 0);
      p->elst = realloc(p->elst, sizeof(struct edge)*new_elsz);
      memset(p->elst + p->elsz, 0x0, sizeof(struct edge)*(new_elsz - p->elsz));
      p->efree = realloc(p->efree, sizeof(uint32_t)*new_elsz);
      for (i = p->elsz; i < new_elsz; i++)
        p->efree[i] = i;
      p->elsz = new_elsz;
    }

    i = p->efree[p->eln++];
    p->elst[i].nxt = p->elst[i].prv = 0xffffffff;
    p->elst[i].x = x;
    p->elst[i].y = y;

    k = adjfind(p->adj, &(p->adjsz), x);
    if (k == 0xffffffff) {
      p->adj = adjinsert(p->adj, &(p->adjsz), &(p->adjn), x, i);
    } else {
      p->elst[i].nxt = p->adj[k].nxt;
      if (p->adj[k].nxt != 0xffffffff)
        p->elst[p->adj[k].nxt].prv = i;
      p->adj[k].nxt = i;
    }

    p->htable =
      hinsert(p->htable, &(p->htsz), &(p->htn), p->elst, i);

    p->ne++;
  }
}

static void
add_link(struct data *p, uint32_t x, uint32_t y) {
  uint32_t i, j, k, l, n;

  if (check_link(p, x, y))
    return;

  if (p->eln < MAXSZ(max(p->nv,p->ne), 0)) {
    add_link_0(p, x, y);
    return;
  }

  /* Let us look for an 'i'... */
  n = MAXSZ(max(p->nv,p->ne), 0);
  for (i = 0; i < p->r; i++) {
    if (p->k2t[i] != NULL)
      n += p->k2t[i]->numberOfEdges;

    if (MAXSZ(max(p->nv,p->ne), i+1) > n + 1)
      break;
  }

  fprintf(stderr, "Rebuilding: %d -> %d/%lf\n", i+1, n+1, MAXSZ(max(p->nv,p->ne), i+1));

  /* Allocate more space? No, we know 'r' as a function of EPS. */
  if (i >= p->r) {
    fprintf(stderr, "Error: collection too big...\n");
    exit(EXIT_FAILURE);
  }

  p->maxr = max(i, p->maxr);

  /* Space for edges in the dynamic structure. */
  uint32_t *xedges = malloc(sizeof(uint32_t)*(p->eln+1));
  uint32_t *yedges = malloc(sizeof(uint32_t)*(p->eln+1));

  /* Load edges in C_0... */
  k = 0;
  for (j = 0; j < p->eln; j++) {
    xedges[k] = p->elst[p->efree[j]].x;
    yedges[k] = p->elst[p->efree[j]].y;
    k++;
  }

	/* Add the new link. */
  xedges[k] = x;
  yedges[k] = y;
  k++;

  assert(k == p->eln+1);

  /* Cleanup C_0. */
  memset(p->elst, 0x0, sizeof(struct edge)*p->elsz);
  for (l = 0; l < p->elsz; l++)
    p->efree[l] = l;
  p->eln = 0;
  memset(p->htable, 0xff, sizeof(uint32_t)*p->htsz);
  p->htn = 0;
  memset(p->adj, 0xff, sizeof(struct adje)*p->adjsz);
  p->adjn = 0;

  /*for (l = 0; l < k; l++)
    printf("%d -> %d\n", xedges[l], yedges[l]); */

  /* Build new C_i. */
  uint32_t max_level = floor(log(p->nv)/log(p->k));
  if(floor(log(p->nv)/log(p->k)) == (log(p->nv)/log(p->k)))
    max_level = max_level-1;
  /* Maybe we should remap nodes and set the number of nodes to the
   * actual number of nodes in the collection. */
  MREP * tmp = compactCreateKTree(xedges, yedges, p->nv, k, max_level);
  /* Initializing missing stuff... Should we keep it always? */
  tmp->div_level_table = p->div_level_table;

  /* Merge and remove old graphs. */
  for (j = 0; j <= i; j++) {
    if (p->k2t[j] != NULL) {
      MREP * old = tmp;

      tmp = k2tree_union(old, p->k2t[j]);
      tmp->div_level_table = p->div_level_table;

      destroyBitRankW32Int(p->k2t[j]->btl);
      free(p->k2t[j]);

      destroyBitRankW32Int(old->btl);
      free(old);
    }
    p->k2t[j] = NULL;
  }

  assert(p->k2t[i] == NULL);
  p->k2t[i] = tmp;

  /* Setup common data structures... */
  p->k2t[i]->info = p->info;
  p->k2t[i]->element = p->element;
  p->k2t[i]->basex = p->basex;
  p->k2t[i]->basey = p->basey;
  p->k2t[i]->iniq = -1;
  p->k2t[i]->finq = -1;
  p->k2t[i]->info2[0] = p->info2[0];
  p->k2t[i]->info2[1] = p->info2[1];

  free(xedges);
  free(yedges);

  p->ne ++;
}

static void
del_link(struct data *p, uint32_t x, uint32_t y) {
  uint32_t k, l;

  k = hfind(p->htable, &(p->htsz), p->elst, x, y);
  if (k != 0xffffffff) {
    p->htable =
      hdelete(p->htable, &(p->htsz), &(p->htn), p->elst, k);
    if (p->elst[k].nxt != 0xffffffff)
      p->elst[p->elst[k].nxt].prv = p->elst[k].prv;
    if (p->elst[k].prv != 0xffffffff)
      p->elst[p->elst[k].prv].nxt = p->elst[k].nxt;
    else {
      l = adjfind(p->adj, &(p->adjsz), p->elst[k].x);
      p->adj[l].nxt = p->elst[k].nxt;
    }

    p->efree[--p->eln] = k;
    p->ne--;
  } else {
    uint32_t ned = 0;
    for (l = 0; l <= p->maxr; l++)
      if (p->k2t[l] !=NULL && compact2MarkLinkDeleted(p->k2t[l], x, y)) {
        p->ne--;

        ned += p->k2t[l]->numberOfMarkedEdges;

        if (p->k2t[l]->numberOfMarkedEdges == p->k2t[l]->numberOfEdges) {
          ned -= p->k2t[l]->numberOfMarkedEdges;
          destroyBitRankW32Int(p->k2t[l]->btl);
          free(p->k2t[l]);
          p->k2t[l] = NULL;
        }
      }

    if (ned > p->ne / TAU(p->ne)) {
      /* Rebuild data structure... */

      /* Let us look for an 'i'... */
      uint32_t n = MAXSZ(max(p->nv,p->ne), 0), i, j;
      for (i = 0; i < p->r; i++) {
        if (p->k2t[i] != NULL)
          n += p->k2t[i]->numberOfEdges;

        if (MAXSZ(max(p->nv,p->ne), i+1) > n + 1)
        break;
      }

      fprintf(stderr, "Rebuilding: %d -> %d/%lf\n", i+1, n+1, MAXSZ(max(p->nv,p->ne), i+1));

      /* Allocate more space? No, we know 'r' as a function of EPS. */
      if (i >= p->r) {
        fprintf(stderr, "Error: collection too big...\n");
        exit(EXIT_FAILURE);
      }

      p->maxr = max(i, p->maxr);

      /* Space for edges in the dynamic structure. */
      uint32_t *xedges = malloc(sizeof(uint32_t)*(p->eln+1));
      uint32_t *yedges = malloc(sizeof(uint32_t)*(p->eln+1));

      /* Load edges in C_0... */
      k = 0;
      for (j = 0; j < p->eln; j++) {
        xedges[k] = p->elst[p->efree[j]].x;
        yedges[k] = p->elst[p->efree[j]].y;
        k++;
      }

    	/* Add the new link. */
      xedges[k] = 1;
      yedges[k] = 1;
      k++;

      assert(k == p->eln+1);

      /* Cleanup C_0. */
      memset(p->elst, 0x0, sizeof(struct edge)*p->elsz);
      for (l = 0; l < p->elsz; l++)
        p->efree[l] = l;
      p->eln = 0;
      memset(p->htable, 0xff, sizeof(uint32_t)*p->htsz);
      p->htn = 0;
      memset(p->adj, 0xff, sizeof(struct adje)*p->adjsz);
      p->adjn = 0;

      /* Build new C_i. */
      uint32_t max_level = floor(log(p->nv)/log(p->k));
      if(floor(log(p->nv)/log(p->k)) == (log(p->nv)/log(p->k)))
        max_level = max_level-1;
      /* Maybe we should remap nodes and set the number of nodes to the
       * actual number of nodes in the collection. */
      MREP * tmp = compactCreateKTree(xedges, yedges, p->nv, k, max_level);
      /* Initializing missing stuff... Should we keep it always? */
      tmp->div_level_table = p->div_level_table;

      /* Merge and remove old graphs. */
      for (j = 0; j <= i; j++) {
        if (p->k2t[j] != NULL) {
          MREP * old = tmp;

          tmp = k2tree_union(old, p->k2t[j]);
          tmp->div_level_table = p->div_level_table;

          destroyBitRankW32Int(p->k2t[j]->btl);
          free(p->k2t[j]);

          destroyBitRankW32Int(old->btl);
          free(old);
        }
        p->k2t[j] = NULL;
      }

      assert(p->k2t[i] == NULL);
      p->k2t[i] = tmp;

      /* Setup common data structures... */
      p->k2t[i]->info = p->info;
      p->k2t[i]->element = p->element;
      p->k2t[i]->basex = p->basex;
      p->k2t[i]->basey = p->basey;
      p->k2t[i]->iniq = -1;
      p->k2t[i]->finq = -1;
      p->k2t[i]->info2[0] = p->info2[0];
      p->k2t[i]->info2[1] = p->info2[1];
      p->k2t[i]->numberOfMarkedEdges = 0;

      free(xedges);
      free(yedges);
    }
  }
}

static int
check_link(struct data *p, uint32_t x, uint32_t y) {
  uint32_t i;

  if (hfind(p->htable, &(p->htsz), p->elst, x, y) != 0xffffffff)
    return 1;

  for (i = 0; i <= p->maxr; i++)
    if (p->k2t[i] !=NULL && compact2CheckLinkQuery(p->k2t[i], x, y))
      return 1;

  return 0;
}

static void
list_neighbors(struct data *p, uint32_t x) {
  uint32_t k, l, n = 0;

  printf("N[%u] ->", x);
  k = adjfind(p->adj, &(p->adjsz), x);
  if (k != 0xffffffff)
    for(k = p->adj[k].nxt; k != 0xffffffff; k = p->elst[k].nxt)
      /* printf(" %u", p->elst[k].y); */
      n ++;

  for (l = 0; l <= p->maxr; l++)
    if (p->k2t[l] != NULL) {
      uint32_t * lst = compact2AdjacencyList(p->k2t[l], x);
      /*for (k = 0; k < lst[0]; k++)
          printf(" %u", lst[k+1]);*/
      n += lst[0];
      free(lst);
    }

  printf("%u\n", n);
}

