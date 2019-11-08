#ifndef __NODEMANAGER_H__

#define __NODEMANAGER_H__

#include "util.h"
#include <stdio.h>


#define EMPTY 	1
#define UNUSED 	2
#define USED 	4
#define DIRTY	8
#define F_DIRTY 16



typedef uint BID;
typedef unsigned char NodeType;

#define NODETYPE(BID) ((BID >> 24) & 0xFF)
#define RAWBID(BID) (BID & 0x00FFFFFF)

#define ADDNODETYPE(N, NT) (N->bid = N->bid | ((NT & 0xFF)<<24))
#define SETNODEBID(bid, nt) (bid = bid | ((nt&0xFF) << 24))

#define BaseNodeType 0

//uint naccesos[MAX_BLOCKS_NM];
//uint ncolisiones;

typedef struct sNode {
	BID bid;
	uint * data;
} Node;


typedef struct sbNodeManager {
	FILE * file;
	char * fileName;
	int fileSize;
	FILE * emptyFile;
	char * emptyFileName;
	uint blockSize;
	uint maxBlockSize;
#ifdef MEMCACHE
	Node **cache;
#else
	uint maxBlocks;
	Node *cache;
	byte *statuscache;
	vector *cachePositions;
//	IntQueue blockFIFO;
#endif
	IntStack emptyBlocksList;
} BasicNodeManager;

typedef struct sNodeManager {
	uint nSizes;
	uint jump;
	uint blockSize;
	BasicNodeManager **delegates;
} NodeManager;


NodeManager * createNodeManager(char * fileName, uint blockSize, uint njumps);

void destroyNodeManager(NodeManager * mgr);

Node * allocateNode(NodeManager * mgr);

Node * allocateANode(NodeManager *mgr, NodeType nt);

Node * allocateAnyNode(NodeManager *mgr);

Node * readNode(NodeManager * mgr, BID bid);

void saveNode(NodeManager * mgr, Node * node);

void releaseNode(NodeManager * mgr, Node * node);

void destroyNode(NodeManager *mgr, Node * node);

uint memUsageMgr(NodeManager * mgr);
uint diskUsageMgr(NodeManager * mgr);

/*private*/
int getFreePosition(BasicNodeManager * mgr, BID bid);
int hasCacheInfo(void);
/*void preLoad(BasicNodeManager * mgr);*/
void preDestroy(BasicNodeManager *mgr);
int getCachePosition(BasicNodeManager *mgr, BID bid);
void setCachePosition(BasicNodeManager *mgr, BID bid, int pos);
void clearCachePosition(BasicNodeManager *mgr, BID id);
void _saveNode(BasicNodeManager *mgr, Node *node);


void writeNode(BasicNodeManager * mgr, int pos);

#endif
