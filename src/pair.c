#include <stdio.h>
#include "pair.h"

PAIR newPair(uint a, uint b){
	PAIR pair;
	pair.a = a;
	pair.b = b;
	return pair;
}