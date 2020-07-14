#ifndef K2TREE_COMMON
#define K2TREE_COMMON



#include "nodeManager.h"
#include "vocUtils.h"

#define LEAF (0x80000000)
#define INTERNAL 0
#define SIZE (0x00ffffff)
#define SAMPLES (0x7f000000)

#define ISLEAF(x) 		((x)&0x80000000)
#define ISINTERNAL(x) 	(!ISLEAF(x))

#define GETSIZE(x)		((x)&SIZE)
#define SETSIZE(x,s)	((x)=((x)&~SIZE)|((s)&SIZE))

#define NSAMPLES(x)		(((x)&SAMPLES)>>24)
#define SETNSAMPLES(x,s)  (((x)=((x)&~SAMPLES)|(((s)<<24)&SAMPLES)))

//#define OFFSETSAMPLES(x)  (0)//(NSAMPLES(x)*16)

/*Operations that can be done*/
#define QUERY 	0
#define SET 	1	/*We are setting a bit to one*/
#define CLEAR	2	/*Setting a bit to zero*/
#define ADD		4	/*ADD is the same as SET, but we will have to add bits*/
#define REMOVE  8	/*REMOVE bits, when its parent is CLEARed*/
#define QUERY_REMOVE 16  /*Get value of current bit and also get if all bits in block are cleared*/

#define ALL_CLEARED 0x80000000

/*Max depth of the B-tree*/
#define MAX_LEVELS 17

#define MAX_KK 256


#define SAMPLESIZEBITS1 16 	//SAMPLESIZE*8
#define SAMPLESIZE1 2		//sample size
#define SAMPLESIZEBITS2 16 	//SAMPLESIZE*8
#define SAMPLESIZE2 2		//sample size
#ifndef SAMPLEPOS
#define SAMPLEPOS 128	  	//sampled interval
#endif
#define SAMPLEPOSBITS  (SAMPLEPOS*8)


//#define SAMPLESIZEBITS 16 	//SAMPLESIZE*8
//#define SAMPLESIZE 2		//sample size
//#define SAMPLEPOSBITS 8192 	//SAMPLEPOS*8
//#define SAMPLEPOS 1024

#define GETNENTRIES(f,n,o) (f?((uint *)(n)->data)[1+(o)*3]:((uint *)(n)->data)[1+(o)*2])
#define GETNONES(f,n,o) (f?((uint *)(n)->data)[1+(o)*3+1]:0)
#define GETBID(f,n,o) (f?((uint *)(n)->data)[1+(o)*3+2]:((uint *)(n)->data)[1+(o)*2+1])

//#define NSAMPLES(n) (BASESAMPLES + NODETYPE(n->bid));

typedef struct sK2Tree {
	FILE * infoFile;		/*Keeps the tree info.*/
	char * infoFileName;
	NodeManager  * mgr;		/*Used to read/write nodes*/
//	Node * root1;			/*Root node. This node is necessary for all operations.*/
//	Node * root2;			/*Root node last level*/
	BID bidroot1;
	BID bidroot2;
	uint depth1;			/*Depth (of the b-tree)*/
	uint depth2;			/*Depth (of the b-tree for last level)*/
	uint nNodes;			/*Number of nodes allowed*/
	uint nNodesReal;		/*Real number of nodes*/
	ull nEdges;				/*Real number of edges*/
	uint nLevels;			/*Levels of the k2-tree*/
	uint * kValues;			/*k values for each level*/
	uint * divLevels;		/*division to apply at each level to get position*/
	ull * startLevels;		/*position where each level starts (real in counter, absolute in positions)*/
	ull * onesBeforeLevel;	/*ones before each level (used in counter)*/
	VOC * voc;				/*leaves vocabullary*/
	byte *PLAYGROUND[2];
	uint useDictionary;
	int inFirstLevels;
	BID curRootBid;
	int debug;
	IntStack emptyNodes;
	uint *nSamples;
	/*Used for retrieving neighbors*/
	ull * positions[2];
	uint * rows[2];
	uint * columns[2];
	uint * ones[2];

	uint * firstFrom[2];
	uint * lastFrom[2];
	uint * firstTo[2];
	uint * lastTo[2];
} K2Tree;

typedef struct internalnodeentry1 {
	uint entries;
	uint ones;
	BID bid;
} INodeEntry1;

typedef struct internalnodeentry2 {
	uint entries;
	BID bid;
} INodeEntry2;

#define ENTRYSIZEBITS1 (sizeof(INodeEntry1)*8)
#define ENTRYSIZEBITS2 (sizeof(INodeEntry2)*8)

#define GETNENTRIES2(k,n) (GETSIZE(n->data[0])/(k->inFirstLevels?ENTRYSIZEBITS1:ENTRYSIZEBITS2))

#define ADJ_LIST 0
#define EDGE_LIST 1

extern int format;

extern Node * path[MAX_LEVELS];
extern uint offsets[MAX_LEVELS];
extern ull initPosLevel[MAX_LEVELS];
extern ull maxPosLevel[MAX_LEVELS];
extern uint acumOnesLevel[MAX_LEVELS];
extern uint depth;


K2Tree * createK2Tree(char *sourceFileName, char *dataFileName, char * vocFileName, uint blockSize, uint njumps, uint valuesLength, uint * values, uint leafLevels);

void destroyK2Tree(K2Tree * k2tree);

K2Tree * loadK2Tree(char * dataFileName, int readonly);

uint findEdge(K2Tree * k2tree, uint nodeFrom, uint nodeTo);

uint insertEdge(K2Tree * k2tree, uint nodeFrom, uint nodeTo);

uint insertEdges(K2Tree * k2tree, uint nodeFrom, uint nNeighbors, uint *neighbors);

uint removeEdge(K2Tree * k2tree, uint nodeFrom, uint nodeTo);

uint *findNeighbors(K2Tree * k2tree, uint nodeFrom);

uint *findRevNeighbors(K2Tree * k2tree, uint nodeTo);

uint **findRange(K2Tree * k2tree, uint lf, uint rf, uint lt, uint rt);

uint insertNode(K2Tree * k2tree);

void deleteNode(K2Tree * k2tree, uint node);

void displayTreeInfo(K2Tree * k2tree);

void * doSomething(K2Tree * k2tree, void (*firstfun)(K2Tree * k2tree), void (*funInternalNodes)(K2Tree *k2tree, Node * internalNode),
		void (*funLeaves)(K2Tree * k2tree, Node * leaf), void *(*lastfun)(K2Tree * k2tree));

/*
 * Check if vocabulary compression is good.
 * If not, rebuild last level of k2tree.
 */
void checkVocabulary(K2Tree * k2tree);

ull memUsageK2Tree(K2Tree * k2tree);
ull diskUsageK2Tree(K2Tree * k2tree);

//Internal use
K2Tree * createEmptyTree(char * fileName, uint nNodes, uint nodesOrig, uint blockSize, uint njumps, uint nLevels, uint *kValues);
void fillInfo(K2Tree * k2tree);
void setRoot(K2Tree *k2tree, BID root);
void loadInfo(K2Tree * k2tree, char * dataFileName);
uint * getKForLevels(uint valuesLength, uint values[], uint *nNodes, uint *nlevels, uint leafLevels);
void rebuildLastLevel(K2Tree * k2tree);


uint findChild(uint * data, uint curSize, ull pos, uint kk, byte operation, ull * _acumOnes, ull * _acumEntries, BID *child);


uint getValues(uint firstLevels, Node * node, ull *entries, ull * ones);

uint doInsertEdge(K2Tree * k2tree, uint nodeFrom, uint nodeTo);

uint *getNeighbors(K2Tree * k2tree, uint nodeFrom, uint nodeTo);

uint **getRange(K2Tree * k2tree, uint lf, uint rf, uint lt, uint rt);

uint getPosition(K2Tree * k2tree, ull *pos, byte operation, uint k2treelevel, uint reset);

void printNode(K2Tree * k2tree, Node * node);

void resetPath(K2Tree * k2tree);


#endif
