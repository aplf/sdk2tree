#include <stdio.h>
#include <string.h>
#include "basic.h"

#ifndef adyList_h
#define adyList_h
#include "adylist.h"
#endif

ALREP * adylistUnionOperation(ALREP * A, ALREP * B);
ALREP * adylistDifferenceOperation(ALREP * A, ALREP * B);
ALREP * adylistIntersectionOperation(ALREP * A, ALREP * B);
ALREP * adylistSymmetricDifferenceOperation(ALREP * A, ALREP * B);
ALREP * adylistComplementOperation(ALREP * A);

