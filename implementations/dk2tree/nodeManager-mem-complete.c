#include "nodeManager.h"
#include <time.h>

ull lecturas = 0LU;
ull accesos = 0LU;
ull saltos = 0LU;
ull escrituras = 0LU;

static inline Node * initSb(BasicNodeManager *mgr, int sb_offset) {
	int i = 0;

	mgr->cache[sb_offset] = (Node *) MALLOC(SB_SIZE * sizeof(Node));
	for (i = 0; i < SB_SIZE; i++) {
		mgr->cache[sb_offset][i].data = NULL;
	}

	return mgr->cache[sb_offset];
}

inline int getCachePosition(BasicNodeManager * mgr, BID bid) {
	return bid;
}

inline void writeNode(BasicNodeManager * mgr, int pos) {
	fseek(mgr->file, pos * mgr->blockSize, SEEK_SET);
	fwrite(mgr->cache[pos/SB_SIZE][pos%SB_SIZE].data, 1, mgr->blockSize, mgr->file);
}

Node * _firstReadNode(BasicNodeManager * mgr, BID id){

	if (!mgr->file) exit(EXIT_UNEXPECTED);
	accesos++;
	Node *n;

	uint node = id;

	Node * sb = mgr->cache[node/SB_SIZE];
	if (!sb) sb = initSb(mgr, node/SB_SIZE);
	n = &(sb[node%SB_SIZE]);
	if (!n->data)
		n->data = (uint *) malloc(mgr->blockSize);

	fseek(mgr->file, id * mgr->blockSize, SEEK_SET);
	n->bid = id;
	uint readres = fread(n->data, 1, mgr->blockSize, mgr->file);
	if (readres != mgr->blockSize) {
		exit(-1);
	}

	return n;
}

BasicNodeManager * _createNodeManager(int blockSize, char * fileName) {
	int i;
	BasicNodeManager *basicManager = (BasicNodeManager *) MALLOC(sizeof(BasicNodeManager));
	char * emptyFileName;

	basicManager->cache = (Node **) MALLOC(N_SB * sizeof(Node *));
	for (i = 0; i < N_SB; i++)  {
		basicManager->cache[i] = NULL;
	}

	basicManager->file = openFile(fileName);

	uint fileSize = 0;
	fseek(basicManager->file, 0, SEEK_END);
	fileSize = ftell(basicManager->file);

	basicManager->fileSize = fileSize;

	fseek(basicManager->file, 0, SEEK_SET);
	basicManager->fileName = fileName;
	basicManager->blockSize = blockSize;

	initSb(basicManager, 0);

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

	uint k;
	int jump = blockSize / njumps;
	char * basicFileName;
	int i;
	for (i = 0; i < 8192; i++)
		fooarray[i] = 0;

	manager -> nSizes = njumps;
	manager->jump = jump;
	manager->blockSize = blockSize;
	manager -> delegates = (BasicNodeManager **) MALLOC ( njumps * sizeof (BasicNodeManager *));

	for (k = 0; k < njumps; k++) {
		basicSize = blockSize + k * jump;
		basicFileName = (char *) MALLOC(256*sizeof(char));
		sprintf(basicFileName, "%s_%d", fileName, k);

		manager->delegates[k] = _createNodeManager(basicSize, basicFileName);
		for (i = 0; i < manager->delegates[k]->fileSize/manager->delegates[k]->blockSize; i++) {
			_firstReadNode(manager->delegates[k], i);
		}
		fclose(manager->delegates[k]->file);
	}
//	free(basicFileName);
//	ncolisiones = 0;
	return manager;
}

int totalsize = 0;
void destroyNodeManager(NodeManager * mgr) {
	int k;
	uint i, j;
	FILE * res;

	printf("Destroying node manager.");
//	DEBUG("lecturas: %ld, accesos:%ld\n", lecturas, accesos);

	for (k = 0; k < mgr->nSizes; k++) {
		{
			int i;
			int lastNode = 0;
			mgr->delegates[k]->file = fopen(mgr->delegates[k]->fileName, "r+");
			for (i = 0; i < N_SB; i++) {
				Node * sb = mgr->delegates[k]->cache[i];
				if (sb) {
					for (j = 0; j < SB_SIZE; j++) {
						if ((i*SB_SIZE + j)*mgr->delegates[k]->blockSize >= mgr->delegates[k]->fileSize) break;
						writeNode(mgr->delegates[k], i*SB_SIZE + j);
					}
				}
			}
			totalsize += (lastNode+1) * mgr->delegates[k]->blockSize;
		}

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

		for (i = 0; i < N_SB; i++) {
			Node * sb = mgr->delegates[k]->cache[i];

			if (sb) {
				for (j = 0; j < SB_SIZE; j++) {
					if (sb[j].data)
						free(sb[j].data);
				}
				free(sb);
			}
		}

		free(mgr->delegates[k]->cache);
		free(mgr->delegates[k]->fileName);
		fclose(mgr->delegates[k]->file);


		free(mgr->delegates[k]);
	}
	free(mgr->delegates);
	free(mgr);

}


Node * _allocateNode(BasicNodeManager * mgr) {
	int node;
	Node *n;
	int si = -1;

	int newBid = 0;

	if (!isEmptyStack(mgr->emptyBlocksList)) {
		newBid = popStack(&(mgr->emptyBlocksList));
	} else {
		si = mgr->fileSize;
		newBid = si/mgr->blockSize;
		mgr->fileSize += mgr->blockSize;
	}

	node = newBid;

	Node * sb = mgr->cache[node/SB_SIZE];
	if (!sb) sb = initSb(mgr, node/SB_SIZE);
	n = &(sb[node%SB_SIZE]);

	if (!n->data) {
		n->data = (uint *)malloc(mgr->blockSize);
	}

	memset(n->data,0,mgr->blockSize);

	n->bid = newBid;


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

static inline Node * _readNode(BasicNodeManager * mgr, BID id) {
	int node;
	lecturas++;
	node = id;

	Node * sb = mgr->cache[node/SB_SIZE];
//	if (!sb) sb = initSb(mgr, node/SB_SIZE);

	return &(sb[node%SB_SIZE]);
}

inline Node * readNode(NodeManager * mgr, BID id) {
	NodeType nt = NODETYPE(id);
	Node *n = _readNode(mgr->delegates[nt], RAWBID(id));
	ADDNODETYPE(n, nt);
	return n;
}

inline void saveNode(NodeManager * mgr, Node *n) {

}

inline void releaseNode(NodeManager * mgr, Node *n) {

}


static inline void _destroyNode(BasicNodeManager * mgr, Node *node) {
	int bid = RAWBID(node->bid);
	pushStack(&(mgr->emptyBlocksList), bid);
}

inline void destroyNode(NodeManager * mgr, Node * node) {
	NodeType nt = NODETYPE(node->bid);
	_destroyNode(mgr->delegates[nt], node);
}

uint memUsageMgr(NodeManager * mgr) {
	uint size = 0;
	uint size2 = 0;
	uint i, j, k;

	size += sizeof(NodeManager) + mgr->nSizes * sizeof(NodeManager *);
	for (i = 0; i < mgr->nSizes; i++) {
		printf("manager %d %d blocks\n", i, mgr->delegates[i]->fileSize / mgr->delegates[i]->blockSize);
		size += sizeof(BasicNodeManager);
		size += N_SB * sizeof(Node *);
		size += mgr->delegates[i]->emptyBlocksList.maxSize * sizeof(uint);
		size += strlen(mgr->delegates[i]->fileName) + strlen(mgr->delegates[i]->emptyFileName);
		for (j = 0; j < N_SB; j++) {
			Node * sb = mgr->delegates[i]->cache[j];
			if (sb) {
				size += SB_SIZE * sizeof(Node);
				for (k = 0; k < SB_SIZE; k++) {
					if (sb[k].data)
						size2 +=mgr->delegates[i]->blockSize;
				}
			}
		}
	}
	INFO("sizes %d %d\n", size, size2);
	return size + size2;
}

uint _diskUsageMgr(BasicNodeManager * mgr) {
	uint res;
	res = mgr->fileSize;
	res += 200; //FIXME: Manter o espacio ocupado pola free-list
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
