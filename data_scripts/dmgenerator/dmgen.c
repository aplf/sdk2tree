
#define _POSIX_C_SOURCE 2

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <assert.h>

#define rand_p() (((double) rand()) / RAND_MAX)
#define rand_n(n) ((int) (((double) (n)) * (rand() / (RAND_MAX + 1.0))))

struct lst {
  unsigned   v;
  struct lst  *nxt;
};

void usage(){
  fprintf(stderr, "Usage: dmgen <int> <double> <init>\n");
}

int
main(int argc, char *argv[])
{
  unsigned nv_arg, i, nv, ne, u, v, w, idstart;
  double p;
  struct timeval t;
  struct lst **adj, *e, *f, *g;
  int *adj_sz, max_dg, *dg;

  if (argc != 4) {
    usage();
    exit(EXIT_FAILURE);
  }

  nv_arg = atoi(argv[1]);
  p = strtod(argv[2], (char **) NULL);
  idstart = atoi(argv[3]);

  fprintf(stderr, "main: args: %d %g\n", nv_arg, p);
  assert(nv_arg > 0 && p >= 0.0 && p <= 1.0);

  gettimeofday(&t, NULL);
  srand((unsigned) (t.tv_sec ^ t.tv_usec));

  adj = (struct lst **) malloc(sizeof(struct lst *)*nv_arg);
  memset(adj, 0, sizeof(struct lst *)*nv_arg);
  adj_sz = (int *) malloc(sizeof(int)*nv_arg);
  memset(adj_sz, 0, sizeof(int)*nv_arg);

  nv = 1;
  ne = 0;

  while (nv < nv_arg) {
    u = rand_n(nv);
    v = nv;
    nv ++;

    printf("a %d %d\n", u, v);
    printf("a %d %d\n", v, u);

    e = (struct lst *) malloc(sizeof(struct lst));
    e->v = v;
    e->nxt = adj[u];
    adj[u] = e;
    adj_sz[u] ++;

    f = (struct lst *) malloc(sizeof(struct lst));
    f->v = u;
    f->nxt = adj[v];
    adj[v] = f; 
    adj_sz[v] ++;

    ne++;

    for(g = adj[u]->nxt; g != NULL; g = g->nxt) {
      if (rand_p() > p)
        continue;

      w = g->v;

      printf("a %d %d\n", v, w);
      printf("a %d %d\n", w, v);

      e = (struct lst *) malloc(sizeof(struct lst));
      e->v = w;
      e->nxt = adj[v];
      adj[v] = e;
      adj_sz[v] ++;
  
      f = (struct lst *) malloc(sizeof(struct lst));
      f->v = v;
      f->nxt = adj[w];
      adj[w] = f; 
      adj_sz[w] ++;

      ne++;
    }
  }

  //printf("x\n");

  fprintf(stderr, "main: graph: %d %d\n", nv, ne);
  assert(nv == nv_arg);

  /*printf("%d %d\n", nv, ne);
  for (i = 0; i < nv_arg; i++)
    for(g = adj[i]; g != NULL; g = g->nxt)
      if (g->v > i)
        printf("%d %d\n", i + idstart, g->v + idstart);*/

  for (i = 0; i < nv_arg; i++) {
    e = adj[i];
    while (e != NULL) {
      g = e;
      e = e->nxt;
      free(g);
    }
  }
  free(adj);

  for (max_dg = i = 0; i < nv_arg; i++)
    if (max_dg < adj_sz[i])
      max_dg = adj_sz[i];

  dg = (int *) malloc(sizeof(int)*(max_dg + 1));
  memset(dg, 0, sizeof(int)*(max_dg + 1));

  for (i = 0; i < nv_arg; i++)
    dg[adj_sz[i]] ++;

  /*
  for (i = 1; i <= max_dg; i++)
    printf("%d:%d\n", i, dg[i]);
  */

  free(adj_sz);
  free(dg);

  return EXIT_SUCCESS;
}

