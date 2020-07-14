#ifndef K2TREE_IMPL
#define K2TREE_IMPL

#include "k2tree-common.h"


Node * goToLeaf(K2Tree * k2tree, uint operation, uint reset, ull pos, ull comparepos, uint offsetPosReal);

void changeLeaf(K2Tree * k2tree, Node ** path, uint * offsets, uint depth,
		uint * newData, uint newDataSizeBits, uint oldDataSizeBits, uint newDataOffsetBits,
		uint newDataEntries, uint oldDataEntries, uint newDataOnes, uint oldDataOnes,
		ull *leafOffset, ull * ones, ull * pos);

extern inline uint countOnes(K2Tree * k2tree, Node *node, int _x, uint * entries);



void alterSamples(K2Tree * k2tree, Node * node, int iniPos, int diffOnes);
void fillSamples(K2Tree * k2tree, Node * node, int iniPos);
uint nLocateEntry(K2Tree * k2tree, Node * node, int nEntry);
uint * getWordSingleOne(uint pos, uint length);

uint findChild1(uint * _data, uint nEntries, ull pos, uint offsetPosReal, byte operation, ull * _acumOnes, ull * _acumEntries, BID * child, ull *_curEntries);
uint findChild2(uint * _data, uint nEntries, ull pos, uint offsetPosReal, byte operation, ull * _curEntries, ull * _acumEntries, BID * child);
void resetPath(K2Tree * k2tree);
Node * getChild(K2Tree * k2tree, Node * internalNode, int child);

int getNSamples(K2Tree * k2tree, int nodeType);

#endif
