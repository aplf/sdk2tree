#include "nodeManager.h"
#include <time.h>


#define MAX_BLOCKS_FILE 10000000
uint MAX_SIZE_NM = 500*1024*1024;


static int seed;
ull lecturas = 0LU;
ull accesos = 0LU;
ull saltos = 0LU;
ull escrituras = 0LU;

inline void writeNode(BasicNodeManager * mgr, int pos) {
//	DEBUG("Writing pos %d\n", pos);
	fseek(mgr->file, RAWBID(mgr->cache[pos].bid) * mgr->blockSize, SEEK_SET);
	fwrite(mgr->cache[pos].data, 1, mgr->blockSize, mgr->file);
}

Node * _readNode(BasicNodeManager * mgr, BID id);

BasicNodeManager * _createNodeManager(int blockSize, char * fileName) {
	int i;
	BasicNodeManager *basicManager = (BasicNodeManager *) MALLOC(sizeof(BasicNodeManager));
	char * emptyFileName;

	basicManager->maxBlocks = MAX_SIZE_NM / blockSize;
	basicManager->cache=(Node *) MALLOC(basicManager->maxBlocks*sizeof(Node));
	basicManager->statuscache=(byte *) MALLOC(basicManager->maxBlocks*sizeof(byte));

	basicManager->file = openFile(fileName);

	uint fileSize = 0;
	fseek(basicManager->file, 0, SEEK_END);
	fileSize = ftell(basicManager->file);

	fseek(basicManager->file, 0, SEEK_SET);
	basicManager->blockSize = blockSize;

	for (i = 0; i < basicManager->maxBlocks; i++) {
		basicManager->cache[i].data = (void *) MALLOC(basicManager->blockSize);
		basicManager->statuscache[i] = EMPTY;
	}


	basicManager->cachePositions = vector_init2(MAX_BLOCKS_FILE);
	for (i = 0; i < MAX_BLOCKS_FILE; i++) {
		vector_set(basicManager->cachePositions, i, -1);
	}

	emptyFileName = (char *) MALLOC(256 * sizeof(char));
	sprintf(emptyFileName, "%s.empty",fileName);

	basicManager->emptyFile = fopen(emptyFileName, "r+");
	basicManager->emptyFileName = emptyFileName;
	initStack(&basicManager->emptyBlocksList, DEF_SMALL_SIZE);
//	initQueue(&basicManager->blockFIFO, basicManager->maxBlocks*2);
//	for (i = 0; i < maxBlocks; i++) {
//		pushQueue(&basicManager->blockFIFO, i);
//	}
	if (basicManager->emptyFile != NULL) {
		fseek(basicManager->file, 0, SEEK_SET);
		while(1) {
			int curBid;
			int readres = fread(&curBid, sizeof(BID), 1 , basicManager->emptyFile);
			if (readres != 1) break;
			pushStack(&(basicManager->emptyBlocksList), readres);
		}
	} else {
		basicManager -> emptyFile = fopen(fileName, "w+");
	}

/*	preLoad(basicManager);*/
	return basicManager;
}

int fooarray[8192];

NodeManager * createNodeManager(char * fileName, uint blockSize, uint njumps) {
	NodeManager * manager = (NodeManager *) MALLOC(sizeof(NodeManager));
	int basicSize = blockSize;
	seed = rand() % MAX_BLOCKS_FILE;
//	seed = 0;
	uint k;
	int jump = blockSize / njumps;
	char * basicFileName = (char *) MALLOC(256*sizeof(char));
	int i;
	for (i = 0; i < 8192; i++)
		fooarray[i] = 0;

	manager -> nSizes = njumps;
	manager->jump = jump;
	manager->blockSize = blockSize;
	manager -> delegates = (BasicNodeManager **) MALLOC ( njumps * sizeof (BasicNodeManager *));

	for (k = 0; k < njumps; k++) {
		basicSize = blockSize + k * jump;
		sprintf(basicFileName, "%s_%d", fileName, k);

		manager->delegates[k] = _createNodeManager(basicSize, basicFileName);

	}
	free(basicFileName);
//	ncolisiones = 0;
	return manager;
}
int notincache = 0;
void destroyNodeManager(NodeManager * mgr) {
	int k;
	uint i;
	FILE * res;

	printf("Destroying node manager.");

	DEBUG("lecturas: %lu, accesos: %lu, escrituras: %lu, saltos :%lu\n", lecturas, accesos, escrituras, saltos);

	for (k = 0; k < mgr->nSizes; k++) {
		DEBUG("mgr %d\n", k);
		preDestroy(mgr->delegates[k]);

		res = freopen(mgr->delegates[k]->emptyFileName, "w+", mgr->delegates[k]->emptyFile);
		if (res == NULL) {
			exit(IO_ERROR);
		}


		while(!isEmptyStack(mgr->delegates[k]->emptyBlocksList)) {
			int value = popStack(&(mgr->delegates[k]->emptyBlocksList));
			fwrite(&value,sizeof(int), 1, mgr->delegates[k]->emptyFile);
		}
		freeStack(mgr->delegates[k]->emptyBlocksList);
		fclose(mgr->delegates[k]->emptyFile);
		free(mgr->delegates[k]->emptyFileName);

		for (i = 0; i < mgr->delegates[k]->maxBlocks; i++) {
			free(mgr->delegates[k]->cache[i].data);
		}

		free(mgr->delegates[k]->cache);
		free(mgr->delegates[k]->statuscache);
		vector_destroy(mgr->delegates[k]->cachePositions);

		fclose(mgr->delegates[k]->file);


		free(mgr->delegates[k]);
	}
	free(mgr->delegates);
	free(mgr);

}


Node * _allocateNode(BasicNodeManager * mgr) {
	int node;
	Node *n = NULL;
	long si = -1;

	int newBid = 0;

	if (!isEmptyStack(mgr->emptyBlocksList)) {
		newBid = popStack(&(mgr->emptyBlocksList));
	} else {

		int res = fseek(mgr->file, 0, SEEK_END);
		if (res)
			exit(IO_ERROR);
		si = ftell(mgr->file);
		if (si < 0 )
			exit(IO_ERROR);
		newBid = si/mgr->blockSize;
	}

	node = getFreePosition(mgr, newBid);

	n = &(mgr->cache[node]);

	memset(n->data,0,mgr->blockSize);

	mgr->statuscache[node] = USED;

	n->bid = newBid;

	fseek(mgr->file, newBid*mgr->blockSize, SEEK_SET);
	fwrite(fooarray, mgr->blockSize, 1, mgr->file);
	setCachePosition(mgr, n->bid, node);
//	if (n->bid == 4) DEBUG("allocated node 4\n");
	return n;
}

inline Node * allocateANode(NodeManager * mgr, NodeType nt) {
	Node * ret = _allocateNode(mgr->delegates[nt]);
	ADDNODETYPE(ret, nt);
	return ret;
}

Node * allocateAnyNode(NodeManager * mgr) {
	int i = 0;
	for (i = mgr->nSizes - 1; i >= 0; i--) {
		if (!isEmptyStack(mgr->delegates[i]->emptyBlocksList))
			return allocateANode(mgr, i);
	}
	return allocateANode(mgr, 0);
}

inline Node * allocateNode(NodeManager * mgr) {
	return allocateANode(mgr, BaseNodeType);
}

inline Node * _readNode(BasicNodeManager * mgr, BID id) {
	int node;
	int readres;

	node = getCachePosition(mgr, id);
	lecturas++;
	if (node == -1) {
		Node *n;

		node = getFreePosition(mgr, id);

		n = &(mgr->cache[node]);
		fseek(mgr->file, id * mgr->blockSize, SEEK_SET);
		n->bid = id;
		readres = fread(n->data, 1, mgr->blockSize, mgr->file);
		setCachePosition(mgr, id, node);
	}
//	DEBUG("reading node %d in position %d\n", mgr->cache[node].bid, node);
//	if (id == 4) DEBUG("read node 4\n");
	if (mgr->statuscache[node] == DIRTY) {
		printf("reading DIRTY node %d\n", id);
	}
	if (mgr->statuscache[node] == F_DIRTY) {
		mgr->statuscache[node] = DIRTY;
	} else {
		mgr->statuscache[node] = USED;
	}

	return &(mgr->cache[node]);
}

inline Node * readNode(NodeManager * mgr, BID id) {
	NodeType nt = NODETYPE(id);
	Node * n = _readNode(mgr->delegates[nt], RAWBID(id));
	ADDNODETYPE(n, nt);
	return n;
}

inline void saveNode(NodeManager * mgr, Node *n) {
	NodeType nt = NODETYPE(n->bid);
//	if (n->bid == 4) DEBUG("saved node 4\n");
//	if (n->bid == 2)
//		DEBUG("saved node 2\n");
	_saveNode(mgr->delegates[nt], n);
}

inline uint _freeNode(BasicNodeManager * mgr, Node * n) {

	int node = getCachePosition(mgr, RAWBID(n->bid));
//	DEBUG("releasing node %d from position %d\n", n->bid, node);
//	if (n->bid == 4) {
//		DEBUG("released node 4\n");
//	}
	if (mgr->statuscache[node] == UNUSED || mgr->statuscache[node] == F_DIRTY) {
		DEBUG("Warning: double release of node. Only allowed if doing parallel searches\n");
		exit(EXIT_UNEXPECTED);
	}
	if (node >= 0) {
		mgr->statuscache[node] = mgr->statuscache[node] == DIRTY ? F_DIRTY: UNUSED;
	}
	return node;
}

inline void releaseNode(NodeManager * mgr, Node * n) {
	NodeType nt = NODETYPE(n->bid);
	_freeNode(mgr->delegates[nt], n);
}


inline void _destroyNode(BasicNodeManager * mgr, Node * node) {
	uint pos;
	pushStack(&(mgr->emptyBlocksList), RAWBID(node->bid));
	pos = vector_get(mgr->cachePositions, RAWBID(node->bid));//_freeNode(mgr, node);
	mgr->statuscache[pos] = EMPTY;
//	if (node->bid == 4) DEBUG("destroyed node 4");
//	DEBUG("destroying node %d from position %d\n", node->bid, pos);
	clearCachePosition(mgr, RAWBID(node->bid));
}

inline void destroyNode(NodeManager * mgr, Node * node) {
	NodeType nt = NODETYPE(node->bid);
	_destroyNode(mgr->delegates[nt], node);
}

uint memUsageMgr(NodeManager * mgr) {
	uint size = 0;
	uint i;
	size += sizeof(struct sbNodeManager);
	for (i = 0; i < mgr->nSizes; i++) {
		size += mgr->delegates[i]->maxBlocks*(sizeof(Node)+mgr->delegates[i]->blockSize);
		DEBUG("size for blocks: %d\n", size);
		size += mgr->delegates[i]->cachePositions->maxSize * sizeof(uint);
		DEBUG("total size: %d\n", size);
	}
	return size;
}

void preDestroy(BasicNodeManager * mgr) {
	int i;
	int lastNode = -1;
//	if(vector_get(mgr->cachePositions, 4) >= 0) {
//		printf("node 4 in position %d, status %d\n", vector_get(mgr->cachePositions, 4), mgr->statuscache[vector_get(mgr->cachePositions, 4)]);
//	}
	for (i = 0; i < mgr->maxBlocks; i++) {
		if (mgr->statuscache[i] == F_DIRTY) {
			writeNode(mgr, i);
			lastNode = i;
		} else if (mgr->statuscache[i] == USED || mgr->statuscache[i] == DIRTY) {
			printf("position %d of cache is in use (BID = %d, status = %d)!\n", i, mgr->cache[i].bid, mgr->statuscache[i] == DIRTY);
			writeNode(mgr, i);
			lastNode = i;
		}
	}
	printf("preDestroy: last node = %d\n", lastNode);
}

inline int hasCacheInfo() {
	return 1;
}

inline int getCachePosition(BasicNodeManager * mgr, BID bid) {
	return vector_get(mgr->cachePositions, bid);
}

inline void setCachePosition(BasicNodeManager * mgr, BID bid, int pos) {
	vector_set(mgr->cachePositions, bid, pos);
}

inline void clearCachePosition(BasicNodeManager *mgr, BID bid) {
	vector_set(mgr->cachePositions, bid, -1);
}

inline void doSaveNode(BasicNodeManager * mgr, Node *n) {
//	DEBUG("Writing node %d\n", n->bid);
	escrituras++;
	fseek(mgr->file, RAWBID(n->bid) * mgr->blockSize, SEEK_SET);
	fwrite(n->data, 1, mgr->blockSize, mgr->file);
}

inline void _saveNode(BasicNodeManager * mgr, Node *n) {
	uint pos = vector_get(mgr->cachePositions, RAWBID(n->bid));
	mgr->statuscache[pos] = DIRTY;
/*	doSaveNode(mgr, n);*/
}


inline int getFreePosition(BasicNodeManager * mgr, BID bid) {
//	while (1) {
//		int slot = popQueue(&mgr->blockFIFO);
//		if (mgr->statusCache[i] == UNUSED || mgr->statusCache[i] == F_DIRTY)
//			return slot;
//	}
	accesos ++;
	uint i;
#ifdef RANDOMCACHE
	int start = (bid + seed) % (mgr->maxBlocks);
	seed = (seed + 15485863) % (mgr->maxBlocks);
#else
	int start = bid % (mgr->maxBlocks);
#endif
	uint njumps = 0;
	uint JUMP = 1;

	for (i = start; njumps++ < mgr->maxBlocks ; i = (i + JUMP) % (mgr->maxBlocks)) {
		saltos++;
		if (mgr->statuscache[i] == EMPTY) return i;
		if (mgr->statuscache[i] == UNUSED || mgr->statuscache[i] == F_DIRTY) {
//			DEBUG("resusing position %d\n", i);
			if (mgr->statuscache[i] == F_DIRTY) {
				doSaveNode(mgr, &(mgr->cache[i]));
				mgr->statuscache[i] = UNUSED;
			}
			if (vector_get(mgr->cachePositions, RAWBID(mgr->cache[i].bid)) == i) {
				vector_set(mgr->cachePositions, RAWBID(mgr->cache[i].bid), -1);
			}
			return i;
		}
	}
	ERROR("No free cache positions!?");
	exit(EXIT_UNEXPECTED);
}



uint _diskUsageMgr(BasicNodeManager * mgr) {
	uint res;
	int foo = fseek(mgr->file, 0, SEEK_END);
	if (foo)
		exit(IO_ERROR);
	res = ftell(mgr->file);
	res += 200; //FIXME: Aproximo o espacio ocupado pola free-list
	return res;
}

uint diskUsageMgr(NodeManager * mgr) {
	uint i;
	uint res = 0;
	for (i = 0; i < mgr->nSizes; i++) {
		res += _diskUsageMgr(mgr->delegates[i]);
	}
	return res;
}
