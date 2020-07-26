#include <stdio.h>
#include <string.h>
#include "Queue.h"
#include "misBits.h"
#include "kTree.h"

/*
	Implementado con la última versión del BitMap que desplaza para concatenar.
*/


MREP * createFromBitmap(misBits * C, int maximalLevel, ulong numNodos, ulong numEdges);
MREP * k2tree_union(MREP * repA, MREP * repB);
MREP * k2tree_symmetricdifference(MREP * repA, MREP * repB);
MREP * k2tree_intersection(MREP * repA, MREP * repB);
MREP * k2tree_difference(MREP * repA, MREP * repB);
MREP * k2tree_complement(MREP * repA);