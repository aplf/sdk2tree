#include "nodeManager.h"

void preDestroy(BasicNodeManager * mgr) {
	int i;
	int lastNode;
	for (i = 0; i < MAX_BLOCKS_NM; i++) {
		if (mgr->cache[i].status == UNUSED) {
			writeNode(mgr, i);
			lastNode = i;
		}
	}
	printf("preDestroy: last node = %d\n", lastNode);
}

inline int hasCacheInfo() {
	return 0;
}

inline int getCachePosition(BasicNodeManager * mgr, BID bid) {
	if (mgr->cache[bid].status == EMPTY) return -1;
	return bid;
}

inline void setCachePosition(BasicNodeManager * mgr, BID bid, int pos) {
	return;
}

inline void clearCachePosition(BasicNodeManager *mgr, BID bid) {
	return;
}

inline void _saveNode(BasicNodeManager * mgr, Node *n) {
	return;
}

inline int getFreePosition(BasicNodeManager * mgr, BID bid) {
	return bid;
}



