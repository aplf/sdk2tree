#include "k2tree-common.h"

extern long calculados;
extern long aforrados;
int format = ADJ_LIST;

uint * getKForLevels(uint valuesLength, uint values[], uint *nNodes, uint *nLevels, uint leafLevels) {
	uint i, j = 0, currentLevel = 0, kValue = 0;
	uint *kValues;
	uint currentSize = 1;
	
	uint levelcounter = 0;
	for (i = 0 ; i < valuesLength; i ++) {
		if (kValue == 0) kValue = values[i];
		else {
			for (j = 0; j < values[i]; j++){
				currentSize *= kValue;
			}
			levelcounter += j;
			kValue = 0;
		}
	}

	for ( ; currentSize < *nNodes; levelcounter++){
		currentSize *= kValue;
	}

	*nLevels = levelcounter - (leafLevels-1);
	*nNodes = currentSize;
	
	kValue = 0;
	currentLevel = 0;
	kValues = (uint *) malloc(*nLevels * sizeof(uint));
	for (i = 0 ; i < valuesLength; i ++) {
		
		if (kValue == 0) kValue = values[i];
		else {
			for (j = 0; j < values[i]; j++){
				kValues[j+currentLevel] = kValue;
			}
			currentLevel += values[i];
			kValue = 0;
		}
	}
	for (j = currentLevel;  j < *nLevels; j++){
		kValues[j] = kValue;
	}
	kValues[*nLevels-1] = pow(kValues[*nLevels-1], leafLevels);

//	for (j = 0; j < *nLevels; j++) {
//		DEBUG("kValues[%d] = %d\n", j, kValues[j]);
//	}
		
	return kValues;
}

K2Tree * loadK2Tree(char * dataFileName, int readonly) {
	uint divLevel;
	int i;
	char *infoFileName = (char *) malloc(256*sizeof(char));
	char *enFileName = (char *) malloc(256*sizeof(char));
	FILE * infoFile;
	K2Tree * k2tree;
	sprintf(infoFileName, "%s", dataFileName);
	infoFile = openFile(infoFileName);

	k2tree = (K2Tree *) malloc(sizeof(K2Tree));
	k2tree->infoFile = infoFile;
	k2tree->infoFileName = infoFileName;

	for (i = 0; i < 2; i++) {
		k2tree->PLAYGROUND[i] = (byte *) malloc(8192 * sizeof(byte));
	}

	k2tree->ones[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->ones[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->rows[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->rows[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->columns[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->columns[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->positions[0] = (ull *) malloc(DEF_BIG_SIZE * sizeof(ull));
	k2tree->positions[1] = (ull *) malloc(DEF_BIG_SIZE * sizeof(ull));
	k2tree->firstFrom[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->firstFrom[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->lastFrom[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->lastFrom[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->firstTo[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->firstTo[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->lastTo[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->lastTo[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));

	loadInfo(k2tree, infoFileName);

	fclose(k2tree->infoFile);
	k2tree->infoFile = NULL;
	initStack(&k2tree->emptyNodes, DEF_SMALL_SIZE);

	sprintf(enFileName, "%s.en", dataFileName);
	FILE * enFile = openFile(enFileName);
	int nEmptyNodes;
	FREAD(&nEmptyNodes, 4, 1, enFile);
	int en;
	for (i = 0; i < nEmptyNodes; i++) {
		FREAD(&en, 4, 1, enFile);
		pushStack(&k2tree->emptyNodes, en);
	}
//	FREAD(k2tree->emptyNodes, , k2tree->nNodes/8, enFile);
	fclose(enFile);
	free(enFileName);

	divLevel = k2tree->nNodes;
	k2tree->divLevels = (uint *) malloc(k2tree->nLevels*sizeof(uint));
	for (i = 0; i < k2tree->nLevels; i++) {
		divLevel=ceil(divLevel/(double)(k2tree->kValues[i]));
		k2tree->divLevels[i] = divLevel;
	}
	if (k2tree->useDictionary) {
		char * vocFileName = (char *) malloc(256*sizeof(char));
		sprintf(vocFileName, "%s.voc", infoFileName);
		k2tree->voc = loadVocabulary(vocFileName, readonly);
		free(vocFileName);
	}

	return k2tree;
}

//uint * mypointer[4];

//uint *mypointer;


K2Tree * createK2Tree(char *sourceFileName, char *dataFileName, char * vocFileName, uint blockSize, uint njumps, uint valuesLength, uint * values, uint leafLevels) {
	int val;
	uint nodes, nodesOrig;
	ull edges;
	int currentNode = -1;
	FILE * sourceFile = fopen(sourceFileName,"r");
	uint * kValues;
	uint nLevels;
	K2Tree * k2tree;
	int readres;
	ull readSize;
	

	readres = fread(&(nodesOrig),sizeof(uint),1,sourceFile);
	if (readres!=1) {
		exit(IO_ERROR);
	}
	uint _edges;
	readres = fread(&(_edges),sizeof(uint),1,sourceFile);
	if (readres!=1) {
		exit(IO_ERROR);
	}
	edges = (ull) _edges;
	nodes = nodesOrig;
	
	kValues = getKForLevels(valuesLength, values, &nodes, &nLevels, leafLevels);

	k2tree = createEmptyTree(dataFileName, nodes, nodesOrig, blockSize, njumps, nLevels, kValues);
	initStack(&k2tree->emptyNodes, DEF_SMALL_SIZE);

	if (k2tree->useDictionary) {
		int wl = k2tree->kValues[k2tree->nLevels-1]*k2tree->kValues[k2tree->nLevels-1]/8;
		k2tree->voc = createEmptyVocabulary(wl);
	}


	readSize = nodesOrig + edges;

	k2tree->debug = 0;

#define BUF_SIZE 67108864

	int *buf = (int *) MALLOC(BUF_SIZE * sizeof(int));

	uint bufPointer = 0;
	int iter;
	uint insedg = 0;

//	printf("max iters: %ld\n", (readSize + BUF_SIZE - 1)/BUF_SIZE);
	for (iter = 0; iter < (readSize + BUF_SIZE - 1)/BUF_SIZE; iter ++) {
//		printf("iter %d\n", iter);
		uint bufSize = fread(buf, sizeof(int), BUF_SIZE, sourceFile);
		
		/* 
		uint readMore = 1;
		if (bufSize < BUF_SIZE) {
			readMore = 0;
		}
		*/
		bufPointer = 0;
		while  (bufPointer < bufSize) {
			switch (format) {
				case EDGE_LIST : {
					uint from = buf[bufPointer];
					uint to = buf[bufPointer+1];
					insertEdge(k2tree, from, to);
					bufPointer += 2;
					break;
				}
				case ADJ_LIST : {
					val = buf[bufPointer];
					if (val < 0) {
						currentNode++;
						bufPointer++;
//						if(!(currentNode % 1000)) {
//							INFO("Current node = %d\n", currentNode);
//						}
					} else {
						uint nNeighbors = 0;
						uint * neighbors = (uint *) buf+bufPointer;
						while(bufPointer < bufSize && buf[bufPointer] >= 0) {
							nNeighbors++;
							bufPointer++;
						}
						if (nNeighbors) {
							insertEdges(k2tree, currentNode, nNeighbors, neighbors);
							insedg += nNeighbors;
						}
					}
					break;
				}
			}
		}
	}
//	printf("%d\n", insedg);
//	printf("%ld %ld\n", calculados, aforrados);
	fclose(sourceFile);
	free(buf);

//	displayTreeInfo(k2tree);
//
//	DEBUG("Rebuilding\n");
//	rebuildLastLevel(k2tree);
//	replaceByOptimal(k2tree->voc);

//	displayTreeInfo(k2tree);
//
/*
	int i;
	uint foo;
	sourceFile = fopen(sourceFileName,"r");
	readres = fread(&(foo),sizeof(uint),1,sourceFile);
	readres = fread(&(foo),sizeof(uint),1,sourceFile);

	DEBUG("Deleting\n");
	currentNode = -1;
	uint readSize2 = readSize * 0.05;
//	uint readSize2 = 1500000;
//	DEBUG("rs2=%d\n", readSize2);
	for  (i  = 0; i < readSize2; i++) {
		readres = fread(&(val), sizeof(uint), 1, sourceFile);
		if (readres!=1) {
			exit(IO_ERROR);
		}

//		if (!(i % 1000)) DEBUG("i = %d\n", i);
		if (val < 0) {
			currentNode++;
//			DEBUG("Current node = %d\n", currentNode);
		} else {
			if (!findEdge(k2tree, currentNode, val-1)) {
				ERROR("pError for edge %d->%d\n", currentNode, val-1);
				exit(EXIT_UNEXPECTED);
			}
			removeEdge(k2tree, currentNode, val-1);

			if (findEdge(k2tree, currentNode, val-1)) {
				ERROR("Error for edge %d->%d\n", currentNode, val-1);
				exit(EXIT_UNEXPECTED);
			}

		}

	}
	fclose(sourceFile);

//	displayTreeInfo(k2tree);

//		DEBUG("Rebuilding\n");
//	rebuildLastLevel(k2tree);
//	replaceByOptimal(k2tree->voc);

	displayTreeInfo(k2tree);

	DEBUG("Inserting\n");
	sourceFile = fopen(sourceFileName,"r");

	readres = fread(&(nodesOrig),sizeof(uint),1,sourceFile);
	if (readres!=1) {
		exit(IO_ERROR);
	}
	readres = fread(&(edges),sizeof(uint),1,sourceFile);
	if (readres!=1) {
		exit(IO_ERROR);
	}

	currentNode = -1;

	for  (i  = 0; i < readSize2; i++) {

		readres = fread(&(val), sizeof(uint), 1, sourceFile);
		if (readres!=1) {
			exit(IO_ERROR);
		}
		if (val < 0) {
			currentNode++;
			if(!(currentNode % 100000)) DEBUG("Current node = %d\n", currentNode);
		} else {
			insertEdge(k2tree, currentNode, val - 1);
			if (!findEdge(k2tree, currentNode, val-1))
				printf("failed for %d->%d\n", currentNode, val-1);
		}

	}
	fclose(sourceFile);
//
////	rebuildLastLevel(k2tree);
////	replaceByOptimal(k2tree->voc);
//
//	displayTreeInfo(k2tree);
*/

	return k2tree;
}

void destroyK2Tree(K2Tree * k2tree) {
	DEBUG("Saving k2tree to disk and freeing allocated memory..\n");

//	if (k2tree->voc->optimalSize != 0) {
//		DEBUG("Size of compressed leaves %d\n", k2tree->voc->compressedSize);
//		DEBUG("Optimal size of compressed leaves %d (rate = %f)\n", k2tree->voc->optimalSize, k2tree->voc->compressedSize*1.0/k2tree->voc->optimalSize);
//	}
//	DEBUG("real compressed size: %d (just checking :P)\n", getCompressedSize(k2tree->voc));
//	DEBUG("real optimal size: %d (just checking :P)\n", getOptimalSize(k2tree->voc));
	int i;
	for (i = MAX_LEVELS - 1; path[i]; i--)
		releaseNode(k2tree->mgr, path[i]);
	displayTreeInfo(k2tree);

	{
		char * enFileName = (char *) malloc(256*sizeof(char));
		sprintf(enFileName, "%s.en", k2tree->infoFileName);
		FILE * enFile = fopen(enFileName, "w+");
		int nn = stackSize(k2tree->emptyNodes);
		fwrite(&nn, 4, 1, enFile);
		int i;
		for (i = 0; i < nn; i++) {
			int cn = popStack(&k2tree->emptyNodes);
			fwrite(&cn, sizeof(int), 1, enFile);
		}
		free(enFileName);
		fclose(enFile);
	}
	k2tree->infoFile = openFile(k2tree->infoFileName);
	fillInfo(k2tree);
	fclose(k2tree->infoFile);

	destroyNodeManager(k2tree->mgr);
	free(k2tree->kValues);
	free(k2tree->divLevels);
	free(k2tree->startLevels);
	free(k2tree->onesBeforeLevel);

	free(k2tree->nSamples);
	freeStack(k2tree->emptyNodes);

	if (k2tree->useDictionary) {
		char * vocFileName = (char *) malloc(256*sizeof(char));
		sprintf(vocFileName, "%s.voc", k2tree->infoFileName);
		saveVocabulary(k2tree->voc, vocFileName);
		free(vocFileName);
		freeVocabulary(k2tree->voc);
	}


	for (i = 0; i < 2; i++) {
		free(k2tree->PLAYGROUND[i]);
	}

	free(k2tree->ones[0]);free(k2tree->rows[0]);free(k2tree->columns[0]);free(k2tree->positions[0]);
	free(k2tree->ones[1]);free(k2tree->rows[1]);free(k2tree->columns[1]);free(k2tree->positions[1]);
	free(k2tree->firstFrom[0]); free(k2tree->lastFrom[0]); free(k2tree->firstTo[0]); free(k2tree->lastTo[0]);
	free(k2tree->firstFrom[1]); free(k2tree->lastFrom[1]); free(k2tree->firstTo[1]); free(k2tree->lastTo[1]);
	free(k2tree->infoFileName);
	free(k2tree);
}

void setRoot(K2Tree *k2tree, BID bid) {
	if (k2tree->inFirstLevels) {
		k2tree->bidroot1 = bid;
		k2tree->curRootBid = bid;
	} else {
		k2tree->bidroot2 = bid;
		k2tree->curRootBid = bid;
	}
}


void fillInfo(K2Tree * k2tree) {
	uint i;
	uint * infoData = (uint *)malloc(2048*sizeof(uint));
	int offsetWrite = 0;

	*(infoData+offsetWrite++)=k2tree->mgr->blockSize;
	*(infoData+offsetWrite++)=k2tree->mgr->nSizes;
	*(infoData+offsetWrite++)=k2tree->nLevels;
	for (i = 0; i < k2tree->nLevels; i++) {
		*(infoData+offsetWrite++)=k2tree->kValues[i];
	}
	*(infoData+offsetWrite++)=k2tree->depth1;
	*(infoData+offsetWrite++)=k2tree->depth2;
	memcpy(infoData+offsetWrite,&(k2tree->bidroot1), sizeof(BID));

	offsetWrite +=sizeof(BID)/4;
	memcpy(infoData+offsetWrite,&(k2tree->bidroot2), sizeof(BID));

	offsetWrite +=sizeof(BID)/4;
	*(infoData+offsetWrite++)=k2tree->nNodes;
	*(infoData+offsetWrite++)=k2tree->nNodesReal;
	*(infoData+offsetWrite++)=k2tree->nEdges;

	for (i = 0; i < k2tree->nLevels; i++) {
		*((ull *)(infoData+offsetWrite))=k2tree->startLevels[i];
		offsetWrite += 2;
	}
	for (i = 0; i < k2tree->nLevels; i++) {
		*((ull *)(infoData+offsetWrite))=k2tree->onesBeforeLevel[i];
		offsetWrite += 2;
	}
	fseek(k2tree->infoFile, 0, SEEK_SET);

	fwrite(infoData,sizeof(uint), offsetWrite, k2tree->infoFile);
	free(infoData);
}

void loadInfo(K2Tree * k2tree, char * dataFileName) {
	uint i;
	uint * infoData;
	int offsetRead = 0;
	int infoSize;
	int njumps, blockSize;

	fseek(k2tree->infoFile, 0, SEEK_END);
	infoSize = ftell(k2tree->infoFile);
	fseek(k2tree->infoFile, 0, SEEK_SET);

	if (infoSize % sizeof(uint)) {
		ERROR("Wrong info file size: %d\n", infoSize);
		exit(EXIT_UNEXPECTED);
	}

	infoData = (uint *) malloc(infoSize);

	FREAD(infoData, sizeof(uint), infoSize / sizeof(uint), k2tree->infoFile);

	blockSize = infoData[offsetRead++];
	njumps = infoData[offsetRead++];

	k2tree->mgr = createNodeManager(dataFileName, blockSize, njumps);
	k2tree->nSamples = (uint *) malloc(njumps*sizeof(uint));
	for (i = 0; i < njumps; i++) {
		if (k2tree->mgr->delegates[i]->blockSize >= SAMPLEPOS)
			k2tree->nSamples[i] = (k2tree->mgr->delegates[i]->blockSize + SAMPLEPOS - 1) / SAMPLEPOS - 1;
		else
			k2tree->nSamples[i] = 0;
	}

	k2tree->nLevels = infoData[offsetRead++];
	k2tree->kValues = (uint *) malloc (k2tree->nLevels * sizeof(uint));
	k2tree->startLevels = (ull *) malloc (k2tree->nLevels * sizeof(ull));
	k2tree->onesBeforeLevel = (ull *) malloc (k2tree->nLevels * sizeof(ull));
	for (i = 0; i < k2tree->nLevels; i++) {
		k2tree->kValues[i] = infoData[offsetRead++];
	}
	k2tree->useDictionary = k2tree->kValues[k2tree->nLevels-1] > k2tree->kValues[k2tree->nLevels-2];
	k2tree->depth1 = infoData[offsetRead++];
	k2tree->depth2 = infoData[offsetRead++];

	memcpy(&k2tree->bidroot1, infoData+offsetRead, sizeof(BID));
//	printNode(k2tree, k2tree->root1);
	offsetRead += sizeof(BID)/4;
	memcpy(&k2tree->bidroot2, infoData+offsetRead, sizeof(BID));
//	printNode(k2tree, k2tree->root2);
	offsetRead += sizeof(BID)/4;

	memcpy(&k2tree->nNodes, infoData+offsetRead, sizeof(uint));
	offsetRead++;
	memcpy(&k2tree->nNodesReal, infoData+offsetRead, sizeof(uint));
	offsetRead ++;
	memcpy(&k2tree->nEdges, infoData+offsetRead, sizeof(int));
	offsetRead ++;
	for (i = 0; i < k2tree->nLevels; i++) {
		k2tree->startLevels[i] = *((ull *)(infoData+offsetRead));
		offsetRead+=2;
	}
	for (i = 0; i < k2tree->nLevels; i++) {
		k2tree->onesBeforeLevel[i] = ((ull *)(infoData+offsetRead))[0];
		offsetRead += 2;
	}
	free(infoData);
}



K2Tree * createEmptyTree(char * fileName, uint nNodes, uint nodesReal,
	uint blockSize, uint njumps, uint nLevels, uint *kValues) {
	uint i, divLevel=nNodes;
	K2Tree * k2tree = (K2Tree *) malloc(sizeof(K2Tree));
	char * infoFileName = (char *) malloc(256*sizeof(char));

	sprintf(infoFileName, "%s", fileName);
	k2tree->infoFile = NULL;//openFile(infoFileName);
	k2tree->infoFileName = infoFileName;

	k2tree->mgr = createNodeManager(fileName, blockSize, njumps);

	k2tree->nSamples = (uint *) malloc(njumps * sizeof(uint));

	for (i = 0; i < njumps; i++) {
		if (k2tree->mgr->delegates[i]->blockSize >= SAMPLEPOS)
			k2tree->nSamples[i] = (k2tree->mgr->delegates[i]->blockSize + SAMPLEPOS - 1) / SAMPLEPOS - 1;
		else
			k2tree->nSamples[i] = 0;
	}

	k2tree->nLevels = nLevels;
	k2tree->kValues = kValues;
	k2tree->divLevels = (uint *)malloc(nLevels*sizeof(uint));
	k2tree->startLevels = (ull *)malloc(nLevels*sizeof(ull));
	k2tree->onesBeforeLevel = (ull *)malloc(nLevels*sizeof(ull));
	k2tree->nNodes = nNodes;
	k2tree->nNodesReal = nodesReal;
	k2tree->nEdges = 0;
	k2tree->depth1 = 1;
	k2tree->depth2 = 1;
	k2tree->useDictionary = k2tree->kValues[k2tree->nLevels-1] > k2tree->kValues[k2tree->nLevels-2];


	for (i = 0; i < nLevels; i++) {
		divLevel=ceil(divLevel/(double)(kValues[i]));
		k2tree->divLevels[i] = divLevel;
		k2tree->startLevels[i] = k2tree->kValues[0]*k2tree->kValues[0];
		k2tree->onesBeforeLevel[i] = 0;
	}

	for (i = 0; i < 2; i++) {
		k2tree->PLAYGROUND[i] = (byte*) malloc(4096 * sizeof(byte));
	}

	k2tree->ones[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->ones[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->rows[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->rows[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->columns[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->columns[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->positions[0] = (ull *) malloc(DEF_BIG_SIZE * sizeof(ull));
	k2tree->positions[1] = (ull *) malloc(DEF_BIG_SIZE * sizeof(ull));
	k2tree->firstFrom[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->firstFrom[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->lastFrom[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->lastFrom[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->firstTo[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->firstTo[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->lastTo[0] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));
	k2tree->lastTo[1] = (uint *) malloc(DEF_BIG_SIZE * sizeof(uint));

	//FIXME: Depends on the internal structure of the tree. Move to gp?
	Node * r1 = allocateANode(k2tree->mgr, 0);
	Node * r2 = allocateANode(k2tree->mgr, 0);
	k2tree->bidroot1 = r1->bid;
	k2tree->bidroot2 = r2->bid;


	uint * infoData;
	uint kk = k2tree->kValues[0]*k2tree->kValues[0];
	infoData = (uint *) r1->data;
	infoData[0] = LEAF | kk;
	for (i = 0; i < kk; i++) {
		bitclean(infoData+1, i);
	}

	r2->data[0] = LEAF;
	//FIXME-end

	k2tree->startLevels[0] = 0;

	saveNode(k2tree->mgr, r1);
	releaseNode(k2tree->mgr, r1);
	saveNode(k2tree->mgr, r2);
	releaseNode(k2tree->mgr, r2);

	return k2tree;

}

#define THRESHOLD 100000
#define MAX_RATE 1
int rebuilt = 0;

void checkVocabulary(K2Tree * k2tree) {
	if (!k2tree->useDictionary) return;
	if (rebuilt) return;
	if (k2tree->voc->compressedSize > THRESHOLD && k2tree->voc->compressedSize > MAX_RATE * k2tree->voc->optimalSize) {
		INFO("Rebuilding last level of k2tree. k2tree has %ld edges\n", k2tree->nEdges);
#ifndef NDEBUG
		int i;
		int positionsLast = (k2tree->onesBeforeLevel[k2tree->nLevels-1]-k2tree->onesBeforeLevel[k2tree->nLevels-2])*k2tree->kValues[k2tree->nLevels-2];
		int * resultados = (int *) calloc(positionsLast/8+1, 1);
		DEBUG("%d positions", positionsLast);
		for (i = 0; i < positionsLast; i++) {
			k2tree->inFirstLevels = 0;
			ull pos = i + k2tree->startLevels[k2tree->nLevels-1];
			int res = getPosition(k2tree, &pos, QUERY, k2tree->nLevels-1, 1);
			if (res)
				bitset(resultados, i);
		}
#endif
		rebuildLastLevel(k2tree);
		replaceByOptimal(k2tree->voc);
		rebuilt = 1;

#ifndef NDEBUG
		for (i = 0; i < MAX_LEVELS; i++) {
			DEBUG("path[%d]=%s(bid %d)\n", i, isNullNode(path[i])?"NULL":"xxx", path[i].bid);
		}
		for (i = 0; i < positionsLast; i++) {
			k2tree->inFirstLevels = 0;
			ull pos = i + k2tree->startLevels[k2tree->nLevels-1];
			int res = getPosition(k2tree, &pos, QUERY, k2tree->nLevels-1, 1);
			ASSERT((res && bitget(resultados, i)) || (!res && !bitget(resultados, i)));
		}
#endif
		INFO("Rebuilt!!\n");
	}
}


uint * findNeighbors(K2Tree * k2tree, uint nodeFrom) {
	return getNeighbors(k2tree, nodeFrom, -1);
}

uint * findRevNeighbors(K2Tree * k2tree, uint nodeTo) {
	return getNeighbors(k2tree, -1, nodeTo);
}

uint ** findRange(K2Tree * k2tree, uint ff, uint lf, uint ft, uint lt) {
	return getRange(k2tree, ff, lf, ft, lt);
}

uint findEdge(K2Tree * k2tree, uint nodeFrom, uint nodeTo) {
	int i;
	uint levelPosition, row, column;
	uint offsetRow = nodeFrom, offsetColumn = nodeTo;

	ull nextPos = 0;
	uint res = 0;
	//ull oldNextPos;

	for (i = 0; i < k2tree->nLevels; i++) {
		uint kk = k2tree->kValues[i] * k2tree->kValues[i];

		if (i > 0) {
			nextPos = (nextPos - k2tree->onesBeforeLevel[i-1]) * kk + k2tree-> startLevels[i];
		}

		row = offsetRow / k2tree->divLevels[i];
		column = offsetColumn / k2tree->divLevels[i];
		levelPosition = row * k2tree->kValues[i] + column;

		nextPos += levelPosition;
		//oldNextPos = nextPos;
//		printf("%ld ", nextPos);
		res = getPosition(k2tree, &nextPos, QUERY, i, i == 0 || i == k2tree->nLevels-1);
		if (!res) {
//			DEBUG("breaking!! %d %ld", i, oldNextPos);
			return 0;
		}
		offsetRow = offsetRow % k2tree->divLevels[i];
		offsetColumn = offsetColumn % k2tree->divLevels[i];
	}
//	printf("\n");
	return res;
}

uint insertEdge(K2Tree * k2tree, uint nodeFrom, uint nodeTo) {
	int i, j;
	uint levelPosition, row, column;
	uint offsetRow = nodeFrom, offsetColumn = nodeTo;

//	TRACE("Inserting %d->%d\n", nodeFrom, nodeTo);
	ull nextPos = 0;
	uint res = 0;
	uint operation = SET;
//	printf("(");
	for (i = 0; i < k2tree->nLevels; i++) {
//		printf("@");
		uint kk = k2tree->kValues[i] * k2tree->kValues[i];
		if (i > 0) {
			ASSERT(nextPos >= k2tree->onesBeforeLevel[i-1]);
			ASSERT(nextPos < k2tree->onesBeforeLevel[i]);
			nextPos = (nextPos - k2tree->onesBeforeLevel[i-1]) * kk + k2tree-> startLevels[i];
		}
		row = offsetRow / k2tree->divLevels[i];
		column = offsetColumn / k2tree->divLevels[i];
		levelPosition = row * k2tree->kValues[i] + column;
		nextPos += levelPosition;

//		ASSERT(nextPos < k2tree->startLevels[i+1] + kk);

//		PRINTIF(nodeFrom == 1796 && nodeTo == 254193, "nextPos=%ld\n", nextPos);
		res = getPosition(k2tree, &nextPos, operation, i, 1);//i==0||i==k2tree->nLevels-1);

//		PRINTIF(nodeFrom == 1796 && nodeTo == 254193, "obtained nextPos=%ld\n", nextPos);
//		res = getPosition(k2tree, &nextPos, operation, i, 1);
		if (operation == SET && res == 0) {
			for (j = i+1; j < k2tree->nLevels; j++) {
				k2tree->onesBeforeLevel[j]++;
			}
			operation = ADD;
		} else if (operation == ADD) {
			if (i + 1 < k2tree->nLevels && k2tree->startLevels[i+1] <= k2tree->startLevels[i])
				k2tree->startLevels[i+1] = k2tree->startLevels[i];
			for (j = i+1; j < k2tree->nLevels; j++) {
				k2tree->onesBeforeLevel[j]++;
				k2tree->startLevels[j] += kk;
			}
		}

		offsetRow = offsetRow % k2tree->divLevels[i];
		offsetColumn = offsetColumn % k2tree->divLevels[i];
	}
//	printf(")");
	if (!res) {
		/*Node was not in graph*/
		k2tree->nEdges++;
	}
	checkVocabulary(k2tree);

	for (i = 0; i < k2tree->nLevels-2; i++) {
		ASSERT((k2tree->onesBeforeLevel[i+1]-k2tree->onesBeforeLevel[i])*k2tree->kValues[i+1]*k2tree->kValues[i+1] == k2tree->startLevels[i+2]-k2tree->startLevels[i+1]);
	}

	return res;
}

#define MAX_NEIGHBORS 100000

uint levelPosition[MAX_NEIGHBORS];
ull nextPos[MAX_NEIGHBORS];
byte operation[MAX_NEIGHBORS];
uint offsetColumn[MAX_NEIGHBORS];

long calculados = 0;
long aforrados = 0;

uint insertEdges(K2Tree * k2tree, uint nodeFrom, uint nNeighbors, uint *neighbors) {
	//int i, j
	int n;
	//uint row;
	//uint offsetRow = nodeFrom;

	if (nNeighbors > MAX_NEIGHBORS)
		exit(MEMORY_EXCEEDED);

	//uint res;
//	uint column;

//	printf("@"); fflush(stdout);
//	printf("-%d-", nodeFrom);fflush(stdout);
	for (n = 0; n < nNeighbors; n++)  {
		insertEdge(k2tree, nodeFrom, neighbors[n]-1);
		ASSERT(findEdge(k2tree, nodeFrom, neighbors[n]-1), "Edge %d->%d not found after insert!!\n", nodeFrom, neighbors[n]-1);
	}

//	for (n = 0; n < nNeighbors; n++)  {
//		operation[n] = SET;
//		offsetColumn[n] = neighbors[n]-1;//FIXME: Cambiado para o meu binary mal feito :S-1;
//		nextPos[n] = 0;
//	}
//	for (i = 0; i < k2tree->nLevels; i++) {
////		DEBUG("level %d\n", i);
//		uint reset = i == 0 || i == k2tree->nLevels - 1;
//		row = offsetRow / k2tree->divLevels[i];
//
//		uint kk = k2tree->kValues[i] * k2tree->kValues[i];
//		if (i > 0) {
//			for (n = 0; n < nNeighbors; n++) {
//				nextPos[n] = (nextPos[n] - k2tree->onesBeforeLevel[i - 1]) * kk
//						+ k2tree-> startLevels[i];
//			}
//		}
//		long lastPos = -1L;
//		uint calcular;
//		for (n = 0; n < nNeighbors; n++) {
//			column = offsetColumn[n] / k2tree->divLevels[i];
//			levelPosition[n] = row * k2tree->kValues[i] + column;
//			nextPos[n] += levelPosition[n];
//			calcular = nextPos[n] != lastPos;
//			if (calcular) {
//				lastPos = nextPos[n];
////				printf(".");
//				res = getPosition(k2tree, &nextPos[n], operation[n], i, reset);
////				res = getPosition(k2tree, &nextPos[n], operation[n], i, 1);
//			} else { //current operation must be SET. Previous operation was SET or ADD. Anyway res will be 1.
//				res = 1;
//				nextPos[n] = nextPos[n-1];
//			}
//
//			reset = 0;
//
//			if (operation[n] == SET && res == 0) {
//				for (j = i + 1; j < k2tree->nLevels; j++) {
//					k2tree->onesBeforeLevel[j]++;
//				}
//				operation[n] = ADD;
//			} else if (operation[n] == ADD) {
//				if (i + 1 < k2tree->nLevels && k2tree->startLevels[i + 1]
//						<= k2tree->startLevels[i])
//					k2tree->startLevels[i + 1] = k2tree->startLevels[i];
//				for (j = i + 1; j < k2tree->nLevels; j++) {
//					k2tree->onesBeforeLevel[j]++;
//					k2tree->startLevels[j] += kk;
//				}
//			}
//
//			offsetColumn[n] = offsetColumn[n] % k2tree->divLevels[i];
//		}
//		offsetRow = offsetRow % k2tree->divLevels[i];
//
//	}
//
//	k2tree->nEdges += nNeighbors;

//	DEBUG("current node %d\n", nodeFrom);
//	if (nodeFrom > 1065 && !findEdge(k2tree,1065,7265177))
//		printf("before");
	checkVocabulary(k2tree);
//	if (nodeFrom > 1065 && !findEdge(k2tree,1065,7265177))
//		printf("after");
	return 0;
}

uint removeEdge(K2Tree * k2tree, uint nodeFrom, uint nodeTo) {
	int i, j;
	uint levelPosition, row, column;
	uint offsetRow = nodeFrom, offsetColumn = nodeTo;

	ull nextPos = 0;
	uint res = 0;
	uint operation = QUERY_REMOVE;
	ull poss[32];
	uint allCleared[32];
	uint cleared[32];
//	DEBUG("removing %d %d\n", nodeFrom, nodeTo);

	/*First pass. We find which levels should be completely removed*/
	for (i = 0; i < k2tree->nLevels; i++) {
		uint kk = k2tree->kValues[i] * k2tree->kValues[i];

		if (i > 0) {
			nextPos = (nextPos - k2tree->onesBeforeLevel[i-1]) * kk + k2tree-> startLevels[i];
		}

		row = offsetRow / k2tree->divLevels[i];
		column = offsetColumn / k2tree->divLevels[i];
		levelPosition = row * k2tree->kValues[i] + column;
		nextPos += levelPosition;
		poss[i] = nextPos;
		res = getPosition(k2tree, &nextPos, operation, i, 1);
		if (!res) {
//			exit(1/0);
			return 0;
		}

		allCleared[i] = res & ALL_CLEARED;

		offsetRow = offsetRow % k2tree->divLevels[i];
		offsetColumn = offsetColumn % k2tree->divLevels[i];
	}

	/*Second pass. We remove or clear levels*/

	for (i = 0; i < 32; i++) cleared[i] = 0;
	for (i = k2tree->nLevels-1; i >= 0; i--) {
		uint kk = k2tree->kValues[i] * k2tree->kValues[i];
		ull rpos;
		//uint auxres;
		rpos = poss[i];

		if (allCleared[i] && (i ==k2tree->nLevels-1 || cleared[i+1])) {
			/* Remove kk bits if all cleared and next level has been cleared*/
			//auxres = getPosition(k2tree, &rpos, REMOVE, i, 1);
			getPosition(k2tree, &rpos, REMOVE, i, 1);
			if (i + 1 < k2tree->nLevels && k2tree->startLevels[i+1] <= k2tree->startLevels[i])
				k2tree->startLevels[i+1] = k2tree->startLevels[i];
			for (j = i+1; j < k2tree->nLevels; j++) {
				k2tree->onesBeforeLevel[j]--;
				k2tree->startLevels[j] -= kk;
//				if (k2tree->startLevels[j] == 0) k2tree->startLevels[j] = k2tree->kValues[0]*k2tree->kValues[0];
			}
			cleared[i] = 1;
		} else if (i == k2tree->nLevels-1 || allCleared[i+1]) {
			/* Clear just one bit if next level has been cleared*/
			//auxres = getPosition(k2tree, &rpos, CLEAR, i, 1);
			getPosition(k2tree, &rpos, CLEAR, i, 1);
			for (j = i+1; j < k2tree->nLevels; j++) {
				k2tree->onesBeforeLevel[j]--;
			}
		} else {
			/*If next level has not been cleared, no changes in this level (neither in upper levels)*/
			break;
		}
	}

	if (res) {
		/*Node was in graph*/
		k2tree->nEdges--;
	}

	checkVocabulary(k2tree);
	return res;
}


uint insertNode(K2Tree * k2tree) {

	if (!isEmptyStack(k2tree->emptyNodes)) {
		uint newNode = popStack(&k2tree->emptyNodes);
		return newNode;
	} else {
		if (k2tree->nNodesReal == k2tree->nNodes) {
			uint * kvalues;
			uint * divlevels;
			ull * startLevels;
			ull * onesBeforeLevel;
			int i;

			kvalues = (uint *)malloc((k2tree->nLevels +1) * sizeof(uint));
			divlevels = (uint *)malloc((k2tree->nLevels +1) * sizeof(uint));
			startLevels = (ull *) malloc((k2tree->nLevels +1) * sizeof(ull));
			onesBeforeLevel = (ull *) malloc((k2tree->nLevels + 1) * sizeof(ull));

			memcpy(kvalues+1, k2tree->kValues, k2tree->nLevels * sizeof(uint));
			kvalues[0] = kvalues[1];
			memcpy(divlevels+1, k2tree->divLevels, k2tree->nLevels * sizeof(uint));
			divlevels[0] = divlevels[1]*kvalues[0];

			//We will add kk bits with 1 one
			startLevels[0] = 0;
			onesBeforeLevel[0] = 0;
			for (i = 0; i < k2tree->nLevels; i++) {
				startLevels[i+1] = k2tree->startLevels[i] + kvalues[0]*kvalues[0];
				onesBeforeLevel[i+1] = k2tree->onesBeforeLevel[i] + 1;
			}

//			int oldNodes = k2tree->nNodes;

			k2tree->nLevels++;
			k2tree->nNodes *= k2tree->kValues[0];

			free(k2tree->kValues);
			free(k2tree->divLevels);
			free(k2tree->startLevels);
			free(k2tree->onesBeforeLevel);
			k2tree->kValues = kvalues;
			k2tree->divLevels = divlevels;
			k2tree->startLevels = startLevels;
			k2tree->onesBeforeLevel = onesBeforeLevel;

			ull pos = 0;
			getPosition(k2tree, &pos, ADD, 0, 1);

		}

		return k2tree->nNodesReal++;
	}
}

void deleteNode(K2Tree * k2tree, uint node) {
	//Mark node as empty
	pushStack(&k2tree->emptyNodes, node);
	//Delete all edges (Direct and reverse neighbors)
	uint * neighbors = getNeighbors(k2tree, node, -1);
	int nn = neighbors[0];
	int i;
	//int res;
	for (i = 0; i < nn; i++) {
		DEBUG("neighbor: %d\n", neighbors[1+i]);
	}
	for (i = 0; i < nn; i++) {
		DEBUG("Removing edge %d->%d\n", node, neighbors[1+i]);
		//res = removeEdge(k2tree, node, neighbors[1+i]);
		removeEdge(k2tree, node, neighbors[1+i]);
		DEBUG("res = %d\n", res);
		DEBUG("Finding edge %d->%d\n", node, neighbors[1+i]);
		if (findEdge(k2tree, node, neighbors[1+i]))
			DEBUG("Failed for direct %d\n", neighbors[1+i]);
	}
	neighbors = getNeighbors(k2tree, -1, node);
	nn = neighbors[0];
	for (i = 0; i < nn; i++) {
		DEBUG("Removing edge %d->%d\n", neighbors[1+i], node);
		//res = removeEdge(k2tree, neighbors[1+i], node);
		removeEdge(k2tree, neighbors[1+i], node);
		DEBUG("res = %d\n", res);
		if (findEdge(k2tree, neighbors[1+i], node))
			DEBUG("Failed for reverse %d\n", neighbors[1+i]);
	}
}




ull memUsageK2Tree(K2Tree * k2tree) {
	/*TODO: complete this size*/
	uint res = 4096 + memUsageMgr(k2tree->mgr);
	INFO("mgr: %d\n", res);
	if (k2tree->useDictionary) res+= memUsageVocabulary(k2tree->voc);
	return res;
}

ull diskUsageK2Tree(K2Tree * k2tree) {
	uint res = diskUsageMgr(k2tree->mgr);
	if (k2tree->useDictionary) res+= diskUsageVocabulary(k2tree->voc);
	return res;
}
