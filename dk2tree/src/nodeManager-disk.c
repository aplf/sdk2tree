#include "nodeManager.h"

extern uint MAX_BLOCKS_NM;

void preDestroy(BasicNodeManager * mgr) {
	int i;
	int lastNode = -1;
	for (i = 0; i < MAX_BLOCKS_NM; i++) {
		if (mgr->cache[i].status == UNUSED) {
			writeNode(mgr, i);
			lastNode = i;
		} else if (mgr->cache[i].status == USED) {
			printf("position %d of cache is in use (BID = %d)!\n", i, mgr->cache[i].node->bid);
		}
	}
	printf("preDestroy: last node = %d\n", lastNode);
}

inline int hasCacheInfo() {
	return 1;
}

inline int getCachePosition(BasicNodeManager * mgr, BID bid) {
	return mgr->cachePositions[bid];
}

inline void setCachePosition(BasicNodeManager * mgr, BID bid, int pos) {
	mgr->cachePositions[bid] = pos;
}

inline void clearCachePosition(BasicNodeManager *mgr, BID bid) {
	mgr->cachePositions[bid] = -1;
}

inline void doSaveNode(BasicNodeManager * mgr, Node *n) {
	fseek(mgr->file, RAWBID(n->bid) * mgr->blockSize, SEEK_SET);
	fwrite(n->data, 1, mgr->blockSize, mgr->file);
}

inline void _saveNode(BasicNodeManager * mgr, Node *n) {
/*	doSaveNode(mgr, n);*/
}
inline int getFreePosition(BasicNodeManager * mgr, BID bid) {
	uint i;
	int start = bid % (MAX_BLOCKS_NM);
	uint njumps = 0;
	uint JUMP = 1;
//	naccesos[start]++;

//	if (bid == 255) {
//		DEBUG("start = %d\n", start);
//		for (i = 0; i < MAX_BLOCKS_NM; i++) {
//			DEBUG("cache[%d] = {status = %d, node = %x}\n", i, mgr->cache[i].status, mgr->cache[i].node);
//			DEBUG("bid = %d\n", RAWBID(mgr->cache[i].node->bid));
//		}
//	}
//	for (i = 0; i < MAX_BLOCKS_NM; i++) {
//		if (mgr->cache[i].node && RAWBID(mgr->cache[i].node->bid) > 10000) {
//			ERROR("Failed\n");
//			return 1/0;
//		}
//	}

	for (i = start; njumps++ < MAX_BLOCKS_NM ; i = (i + JUMP) % (MAX_BLOCKS_NM)) {
		if (mgr->cache[i].status == EMPTY) return i;
		if (mgr->cache[i].status == UNUSED) {
			doSaveNode(mgr, mgr->cache[i].node);
			if (mgr->cachePositions[RAWBID(mgr->cache[i].node->bid)] == i) {
				mgr->cachePositions[RAWBID(mgr->cache[i].node->bid)] = -1;
			}
			return i;
		}
//		ncolisiones++;
	}
	ERROR("No free cache positions!?");
	exit(EXIT_UNEXPECTED);
//	return -1;
}

