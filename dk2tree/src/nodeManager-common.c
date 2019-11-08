#include "nodeManager.h"
#include <time.h>

uint MAX_BLOCKS_NM = 0;

inline void writeNode(BasicNodeManager * mgr, int pos) {
	fseek(mgr->file, RAWBID(mgr->cache[pos].node->bid) * mgr->blockSize, SEEK_SET);
	fwrite(mgr->cache[pos].node->data, 1, mgr->blockSize, mgr->file);
}

Node * _readNode(BasicNodeManager * mgr, BID id);

BasicNodeManager * _createNodeManager(int blockSize, char * fileName) {
	int i;
	BasicNodeManager *basicManager = (BasicNodeManager *) MALLOC(sizeof(BasicNodeManager));
	char * emptyFileName;

	basicManager->cache=(InfoNode *) MALLOC(MAX_BLOCKS_NM*sizeof(InfoNode));

	basicManager->file = openFile(fileName);

	uint fileSize = 0;
	fseek(basicManager->file, 0, SEEK_END);
	fileSize = ftell(basicManager->file);

	fseek(basicManager->file, 0, SEEK_SET);
	basicManager->blockSize = blockSize;

	for (i = 0; i < MAX_BLOCKS_NM; i++) {
		basicManager->cache[i].node = NULL;
		basicManager->cache[i].node = (Node *) MALLOC(sizeof(Node));
		basicManager->cache[i].node->data = (void *) MALLOC(basicManager->blockSize);
/*		basicManager->cache[i].node->data = NULL;*/
		basicManager->cache[i].status = EMPTY;
	}



	if (hasCacheInfo()) {
		basicManager->cachePositions = (int *) MALLOC(MAX_BLOCKS_FILE*sizeof(int));
		for (i = 0; i < MAX_BLOCKS_FILE; i++) {
			basicManager->cachePositions[i] = -1;
		}
/*		emptyQueue(&basicManager->freeBlocks, MAX_BLOCKS);*/
/*		for (i = 0; i < MAX_BLOCKS; i++) {*/
/*			pushQueue(&basicManager->freeBlocks, i);*/
/*		}*/
	}

//	for (i = 0; i < fileSize / basicManager->blockSize; i++) {
//		_readNode(basicManager, i);
////		printf("%d\n", ((uint *)n->data)[0]);
//	}

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

/*	preLoad(basicManager);*/
	return basicManager;
}

int fooarray[8192];

NodeManager * createNodeManager(char * fileName, uint blockSize, uint njumps) {
	NodeManager * manager = (NodeManager *) MALLOC(sizeof(NodeManager));
	int basicSize = blockSize;

	MAX_BLOCKS_NM = 2*1024*1024 / blockSize;

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

		for (i = 0; i < MAX_BLOCKS_NM; i++) {
			if (mgr->delegates[k]->cache[i].node) {
				free(mgr->delegates[k]->cache[i].node->data);
				free(mgr->delegates[k]->cache[i].node);
			}
		}

		free(mgr->delegates[k]->cache);
		if (hasCacheInfo()) {
			free(mgr->delegates[k]->cachePositions);
/*			freeQueue(mgr->delegates[k]->freeBlocks);*/
		}
		fclose(mgr->delegates[k]->file);


		free(mgr->delegates[k]);
	}
	free(mgr->delegates);
	free(mgr);
//	{
//		uint totalaccesos = 0;
//		for (i = 0; i < MAX_BLOCKS_NM; i++) {
//	/*		printf("accesos[%d] = %d\n", i, naccesos[i]);*/
//			totalaccesos += naccesos[i];
//		}
//		printf("colisiones: %d, en %d accesos\n", ncolisiones, totalaccesos);
//		printf("not in cache: %d\n",notincache);
//	}

}


Node * _allocateNode(BasicNodeManager * mgr) {
	int node;
	Node *n = NULL;
	int si = -1;

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

	n = mgr->cache[node].node;

//	if (!n) {
//		mgr->cache[node].node = (Node *)MALLOC(sizeof(Node));
//		if (mgr->cache[node].node == NULL) {
//			ERROR("MALLOC failed!\n");
//		}
//
//		n = mgr->cache[node].node;
//		n->data = (uint *)MALLOC(mgr->blockSize);
//	}
	memset(n->data,0,mgr->blockSize);

	mgr->cache[node].status = USED;

	n->bid = newBid;

//	if (si != -1 && newBid == si/mgr->blockSize) {
		fseek(mgr->file, newBid*mgr->blockSize, SEEK_SET);
//		for (i = 0; i < mgr -> blockSize; i++)
//			fwrite(&foo, 1, 1, mgr->file);
		fwrite(fooarray, mgr->blockSize, 1, mgr->file);
//	}
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

	if (node == -1) {
		Node *n;
//		notincache++;

		node = getFreePosition(mgr, id);

		n = mgr->cache[node].node;
//		if (!n) {
//			mgr->cache[node].node = (Node *) MALLOC(sizeof(Node));
//			n = mgr->cache[node].node;
//			n->data = (uint *)MALLOC(mgr->blockSize);
//		}
		fseek(mgr->file, id * mgr->blockSize, SEEK_SET);
		n->bid = id;
		readres = fread(n->data, 1, mgr->blockSize, mgr->file);
		setCachePosition(mgr, id, node);
	}

	mgr->cache[node].status = USED;

	return mgr->cache[node].node;
}

inline Node * readNode(NodeManager * mgr, BID id) {
//	DEBUG("read %d\n", id);
//	fflush(stdout);
//	exit(1/0);
	NodeType nt = NODETYPE(id);
	Node * n = _readNode(mgr->delegates[nt], RAWBID(id));
	ADDNODETYPE(n, nt);
	return n;
}

inline void saveNode(NodeManager * mgr, Node *n) {
//	DEBUG("save %d\n", n->bid);
//	fflush(stdout);
	NodeType nt = NODETYPE(n->bid);
	_saveNode(mgr->delegates[nt], n);
}

inline uint _freeNode(BasicNodeManager * mgr, Node * n) {
	int node = getCachePosition(mgr, RAWBID(n->bid));
/*	if (hasCacheInfo()) {*/
/*		pushQueue(&mgr->freeBlocks, node);*/
/*	}*/
/*	printf("%d\n", node);*/
	if (node >= 0) {
		mgr->cache[node].status = UNUSED;
	} else {
		DEBUG("Releasing unused node: %d\n", RAWBID(n->bid));
		exit(EXIT_UNEXPECTED);
	}
	return node;
}

inline void _freeNode2(BasicNodeManager * mgr, Node * n) {
//	int node = getCachePosition(mgr, RAWBID(n->bid));
/*	if (hasCacheInfo()) {*/
/*		pushQueue(&mgr->freeBlocks, node);*/
/*	}*/
/*	printf("%d\n", node);*/
//	mgr->cache[node].status = UNUSED;
}

inline void releaseNode(NodeManager * mgr, Node * n) {
//	DEBUG("release %d\n", n->bid);
//	fflush(stdout);
//	exit(1/0);
//	if (n->bid == 3509) exit(1/0);
	NodeType nt = NODETYPE(n->bid);
/*	printf("%d %llx, %llx\n", nt, mgr->delegates[nt], n);*/
	_freeNode(mgr->delegates[nt], n);
}


inline void _destroyNode(BasicNodeManager * mgr, Node * node) {
	uint pos;
	pushStack(&(mgr->emptyBlocksList), RAWBID(node->bid));
	pos = vector_get(mgr->cachePositions, RAWBID(node->bid));//_freeNode(mgr, node);
	mgr->cache[pos].status = EMPTY;
	clearCachePosition(mgr, RAWBID(node->bid));
}

inline void destroyNode(NodeManager * mgr, Node * node) {
	NodeType nt = NODETYPE(node->bid);
//	if (nt == 0 && RAWBID(node->bid)==14) {
//		int i = 0;
//	}
	_destroyNode(mgr->delegates[nt], node);
}

uint memUsageMgr(NodeManager * mgr) {
	uint size = 0;
	uint i;
	size += sizeof(struct sbNodeManager);
	for (i = 0; i < mgr->nSizes; i++) {
		size += MAX_BLOCKS_NM*(sizeof(InfoNode)+mgr->delegates[i]->blockSize);
		if (hasCacheInfo())
			size += MAX_BLOCKS_FILE * sizeof(uint);
	}
	return size;
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
