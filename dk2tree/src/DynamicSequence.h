#include "nodeManager.h"

typedef uint * DSBlock;

extern uint nIntsInternalEntry[];
extern uint nBitsInternalEntry[];

#define MAX_DEPTH 17

typedef struct sDynamicSequence {
	ull curSize;
	BID root;
	uint depth;
	NodeManager *nodeManager;
//	uint *(*changeLeaf(Node *leaf, uint * newData, uint newDataSize, uint oldDataSize));
//	uint (*splitLeaf(uint *data, uint dataSize, uint **leaf1, uint * leaf1Size, uint **leaf2, uint * leaf2Size));
//	uint (*mergeLeaf(uint * data1, uint dataSize1, uint *data2, uint dataSize2, uint **mergeLeaf, uint * mergeLeafSize));
	Node *path[MAX_DEPTH];
	uint offsets[MAX_DEPTH];
	ull acumOnes[MAX_DEPTH];
	ull initSize[MAX_DEPTH];
	ull maxSize[MAX_DEPTH];
	byte rank;
} *DynamicSequence;

typedef struct sDynamicSequenceRank {
	ull curSize;
	ull curOnes;
	BID root;
	uint depth;
	NodeManager *mgr;
//	uint *(*changeLeaf(uint *leafData, uint leafSize, uint * newData, uint newDataSize, uint oldDataSize));
//	uint (*splitLeaf(uint *data, uint dataSize, uint **leaf1, uint * leaf1Size, uint **leaf2, uint * leaf2Size));
//	uint (*mergeLeaf(uint * data1, uint dataSize1, uint *data2, uint dataSize2, uint **mergeLeaf, uint * mergeLeafSize));
	Node *path[MAX_DEPTH];
	uint offsets[MAX_DEPTH];
	ull acumOnes[MAX_DEPTH];
	ull initPosLevel[MAX_DEPTH];
	ull maxPosLevel[MAX_DEPTH];
	byte rank;
} *DynamicSequenceRank;


DynamicSequenceRank DSRCreateEmpty(uint blockSize, uint njumps, uint *(*changeLeaf(uint *, uint, uint *, uint, uint)),
			uint (*splitLeaf(uint *, uint, uint **, uint *, uint **, uint *)),
			uint *(mergeLeaf(uint *, uint, uint *, uint, uint **, uint *)));
//ull DSRRank1(DynamicSequenceRank bs, ull pos);
//ull DSRRank0(DynamicSequenceRank bs, ull pos);
//ull DRSelect1(DynamicSequenceRank bs, ull ones);
//ull DBSRSelect0(DynamicSequenceRank bs, ull ones);
//ull DSRAccess(DynamicSequenceRank bs, ull pos);
//void DSRInsert(DynamicSequenceRank bs, ull pos, uint * data, ull len);
//void DSRDelete(DynamicSequenceRank bs, ull pos, ull len);
//void DSRReplace(DynamicSequenceRank bs, ull pos, uint * data, ull len);

//DynamicBitmap DBSCreateEmpty();
//ull DBSAccess(DynamicBitmap bs, ull pos);
//void DBSInsert(DynamicBitmap bs, ull pos, uint * data, ull len);
//void DBSDelete(DynamicBitmap bs, ull pos, ull len);
//void DBSReplace(DynamicBitmap bs, ull pos, uint * data, ull len);

