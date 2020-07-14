#include "nodeManager.h"


#define MAX_BLOCKS_FILE 10000000
//uint MAX_SIZE_NM = 13800000; //indochina-10%
//uint MAX_SIZE_NM = 38400000; //indochina-30%
//uint MAX_SIZE_NM = 69000000; //indochina-50%
//uint MAX_SIZE_NM = 107400000; //indochina-80%
//uint MAX_SIZE_NM = 28000000; //uk-10%
//uint MAX_SIZE_NM = 84000000; //uk-30%
//uint MAX_SIZE_NM = 200000000; //uk-+-70%
//uint MAX_SIZE_NM = (512*600);
uint MAX_SIZE_NM = 331000000; //huk-10%
//uint MAX_SIZE_NM = 993000000; //huk-30%
//uint MAX_SIZE_NM = 2000000000; //huk-60%
//uint MAX_SIZE_NM = 3400000000; //huk-80%



ull lecturas = 0LU;
ull accesos = 0LU;
ull saltos = 0LU;
ull escrituras = 0LU;

inline void writeNode(BasicNodeManager * mgr, int pos) {
	fseek(mgr->file, (long) RAWBID(mgr->cache[pos].bid) * mgr->blockSize, SEEK_SET);
	fwrite(mgr->cache[pos].data, 1, mgr->blockSize, mgr->file);
}

Node * _readNode(BasicNodeManager * mgr, BID id);
uint _freeNode(BasicNodeManager * mgr, Node * node);

BasicNodeManager * _createNodeManager(int blockSize, char * fileName) {
	int i;
	BasicNodeManager *basicManager = (BasicNodeManager *) MALLOC(sizeof(BasicNodeManager));
	char * emptyFileName;

	basicManager->maxBlocks = MAX_SIZE_NM / blockSize;
	basicManager->cache=(Node *) MALLOC(basicManager->maxBlocks*sizeof(Node));
	basicManager->statuscache=(byte *) MALLOC(basicManager->maxBlocks*sizeof(byte));

	basicManager->file = openFile(fileName);

	ull fileSize = 0;
	fseek(basicManager->file, 0, SEEK_END);
	fileSize = ftell(basicManager->file);

	rewind(basicManager->file);
	basicManager->blockSize = blockSize;

	for (i = 0; i < basicManager->maxBlocks; i++) {
		basicManager->cache[i].data = (void *) MALLOC(basicManager->blockSize);
		basicManager->statuscache[i] = EMPTY;
	}


	basicManager->cachePositions = (uint *) malloc(MAX_BLOCKS_FILE * sizeof(uint));

	basicManager->lru = initLRU(basicManager->maxBlocks);

	for (i = 0; i < MAX_BLOCKS_FILE; i++) {
		basicManager->cachePositions[i] = -1;
	}

	accesos = 0LU;
	lecturas = 0LU;
	emptyFileName = (char *) MALLOC(256 * sizeof(char));
	sprintf(emptyFileName, "%s.empty",fileName);

	basicManager->emptyFile = fopen(emptyFileName, "r+");
	basicManager->emptyFileName = emptyFileName;
	initStack(&basicManager->emptyBlocksList, DEF_SMALL_SIZE);

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


	return basicManager;
}

int fooarray[8192];

NodeManager * createNodeManager(char * fileName, uint blockSize, uint njumps) {
	NodeManager * manager = (NodeManager *) MALLOC(sizeof(NodeManager));
	int basicSize = blockSize;
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
		free(mgr->delegates[k]->cachePositions);

		fclose(mgr->delegates[k]->file);


		free(mgr->delegates[k]);
	}
	free(mgr->delegates);
	free(mgr);

}


Node * _allocateNode(BasicNodeManager * mgr) {
	int node;
	Node *n = NULL;
	ull si = -1;

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

	fseek(mgr->file, (long)newBid*mgr->blockSize, SEEK_SET);
	fwrite(fooarray, mgr->blockSize, 1, mgr->file);
	setCachePosition(mgr, n->bid, node);

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
		fseek(mgr->file, (long) id * mgr->blockSize, SEEK_SET);
		n->bid = id;
		readres = fread(n->data, 1, mgr->blockSize, mgr->file);
		setCachePosition(mgr, id, node);
	} else {
		extractPositionLRU(mgr->lru, node);
	}
/*
	if (mgr->statuscache[node] == DIRTY) {
		printf("reading DIRTY node %d\n", id);
		exit(EXIT_UNEXPECTED);
	}
*/
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
	_saveNode(mgr->delegates[nt], n);
}

inline uint _freeNode(BasicNodeManager * mgr, Node * n) {

	int node = getCachePosition(mgr, RAWBID(n->bid));

	if (mgr->statuscache[node] == UNUSED || mgr->statuscache[node] == F_DIRTY) {
		DEB0("Warning: double release of node. Only allowed if doing parallel searches\n");
		exit(EXIT_UNEXPECTED);
	}

	if (node >= 0) {
		mgr->statuscache[node] = mgr->statuscache[node] == DIRTY ? F_DIRTY: UNUSED;
	}

	pushLRU(mgr->lru, node);
	return node;
}

inline void releaseNode(NodeManager * mgr, Node * n) {
	NodeType nt = NODETYPE(n->bid);
	_freeNode(mgr->delegates[nt], n);
}


inline void _destroyNode(BasicNodeManager * mgr, Node * node) {
	uint pos;
	pushStack(&(mgr->emptyBlocksList), RAWBID(node->bid));
	pos = mgr->cachePositions[RAWBID(node->bid)];//_freeNode(mgr, node);
	mgr->statuscache[pos] = EMPTY;
	pushLRU(mgr->lru, pos);
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
		size += MAX_BLOCKS_FILE * sizeof(uint);
		DEBUG("total size: %d\n", size);
	}
	return size;
}

void preDestroy(BasicNodeManager * mgr) {
	int i;
	int lastNode = -1;
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
	return mgr->cachePositions[bid];
}

inline void setCachePosition(BasicNodeManager * mgr, BID bid, int pos) {
	mgr->cachePositions[bid]=pos;
}

inline void clearCachePosition(BasicNodeManager *mgr, BID bid) {
	mgr->cachePositions[bid]=-1;
}

inline void doSaveNode(BasicNodeManager * mgr, Node *n) {
	escrituras++;
	fseek(mgr->file, (long) RAWBID(n->bid) * mgr->blockSize, SEEK_SET);
	fwrite(n->data, 1, mgr->blockSize, mgr->file);
}

inline void _saveNode(BasicNodeManager * mgr, Node *n) {
	uint pos = mgr->cachePositions[RAWBID(n->bid)];
	mgr->statuscache[pos] = DIRTY;
}


inline int getFreePosition(BasicNodeManager * mgr, BID bid) {
	accesos ++;
	int i = popLRU(mgr->lru);

	if (mgr->statuscache[i] == EMPTY) return i;
	if (mgr->statuscache[i] == UNUSED || mgr->statuscache[i] == F_DIRTY) {
		if (mgr->statuscache[i] == F_DIRTY) {
			doSaveNode(mgr, &(mgr->cache[i]));
			mgr->statuscache[i] = UNUSED;
		}
		if (mgr->cachePositions[RAWBID(mgr->cache[i].bid)] == i) {
			mgr->cachePositions[RAWBID(mgr->cache[i].bid)] = -1;
		}
		return i;
	}
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
