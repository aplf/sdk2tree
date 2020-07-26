#include "DynamicSequence.h"


uint nIntsInternalEntry[] = {2, 3};
uint nBitsInternalEntry[] = {64, 96};

DynamicSequenceRank DSRCreateEmpty(NodeManager *manager, byte rank) {
	DynamicSequenceRank bs = (DynamicSequenceRank) malloc(sizeof(sDynamicSequenceRank));
	bs->mgr = manager;
	Node * node = allocateNode(mgr);
	bs->root = node->bid;
	bs->curSize = 0L;
	bs->rank = rank;
	return bs;
}



//ull DSRRank1(DynamicSequenceRank bs, ull pos) {
//	return -1L;
//}
//
//ull DSRRank0(DynamicSequenceRank bs, ull pos) {
//	return pos - DBSRRank1(bs, pos);
//}
//
//ull DSRSelect1(DynamicSequenceRank bs, ull ones) {
//	return -1L;
//}
//
//ull DSRSelect0(DynamicSequenceRank bs, ull zeros) {
//	return -1L;
//}
//
//ull DSRAccess(DynamicSequenceRank bs, ull pos) {
//	return -1L;
//}
//
//void DSRInsert(DynamicSequenceRank bs, ull pos, uint * data, ull len) {
//	goToLeaf(bs, pos, 1);
//
//}
//
//void DSRDelete(DynamicSequenceRank bs, ull pos, ull len) {
//
//}
//
//void DSRReplace(DynamicSequenceRank bs, ull pos, uint * data, ull len) {
//
//}

//DynamicSequence DBSCreateEmpty(uint (*fun(void *)));
//DynamicSequence DBSCreate(uint * bits, ull length, uint blockSize, uint njumps, uint (*fun(void *)));
//ull DBSAccess(DynamicSequence bs, ull pos);
//void DBSInsert(DynamicSequence bs, ull pos, uint * data, ull len);
//void DBSDelete(DynamicSequence bs, ull pos, ull len);
//void DBSReplace(DynamicSequenceRank bs, ull pos, uint * data, ull len);



