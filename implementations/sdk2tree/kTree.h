#ifndef KTREE_H
#define KTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "bitrankw32int.h"
#include "basic.h"
#define MAX_INFO 1024*1024+10



typedef struct matrixRep
{
    bitRankW32Int * btl;        //Bitmap representando T:L
    uint btl_len;               //Numero de bits de T:L
    uint bt_len;                //Numero de bits de T
    int maxLevel;           //Nivel maximo del arbol
    uint numberOfNodes;
    ulong numberOfEdges;
    ulong numberOfMarkedEdges;
    uint * div_level_table;
    uint * info;
    uint * info2[2];
    uint * element;
    uint * basex;
    uint * basey;
    int iniq;
    int finq;
}MREP;


#define K 2

MREP * compactCreateKTree(uint * xedges, uint *yedges, uint numberOfNodes,ulong numberOfEdges, uint maxl);


uint * compactAdjacencyList(MREP * rep, int x);
uint * compact2AdjacencyList(MREP * rep, int x);
uint * compactInverseList(MREP * rep, int y);
uint ** compactRangeQuery(MREP * rep, uint p1, uint p2, uint q1, uint q2);
uint compactCheckLinkQuery(MREP * rep, uint p, uint q);
uint compact2CheckLinkQuery(MREP * rep, uint p, uint q);
uint compactCheckRangeQuery(MREP * rep, uint p1, uint p2, uint q1, uint q2);
uint compact2MarkLinkDeleted(MREP * rep, uint p, uint q);

int * compactFullDecompression(MREP * rep);

void edgeIterator(MREP * rep, int (*proc)(uint32_t u, uint32_t v));

MREP * loadRepresentation(char * basename);
void saveRepresentation(MREP * rep, char * basename);
void destroyRepresentation(MREP * rep);


#endif
