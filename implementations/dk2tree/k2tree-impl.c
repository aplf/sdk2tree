#include "k2tree-impl.h"
#include "encoding.h"

uint PLAYGROUND[256];

Node * path[MAX_LEVELS];
uint offsets[MAX_LEVELS];
ull initPosLevel[MAX_LEVELS];
ull maxPosLevel[MAX_LEVELS];
uint acumOnesLevel[MAX_LEVELS];
uint depth;




int getNSamples(K2Tree * k2tree, int nodeType) {
	if (!k2tree->inFirstLevels && !k2tree->useDictionary) return 0;
	return k2tree->nSamples[nodeType];
}

uint getSampleValue(uint * data, int nSample, int ss) {
	switch(ss) {
		case 1 : return ((byte *)(data+1))[nSample];
		case 2 : return GetFieldW16(data+1, nSample);
		default: return GetField(data+1, ss*8, nSample);
	}

}

uint mask8[4] = {0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF, 0x00FFFFFF};
uint mask16[2] = {0xFFFF0000, 0x0000FFFF};

void setSampleValue(uint *data, int nSample, int value, int ss) {
	switch(ss) {
		case 1 :
		{
			uint pos = 1 + nSample / 4;
			uint mod = nSample%4;
			data[pos] = (data[pos] & mask8[mod]) | (value << 8*mod);
			break;
		}
		case 2 :
		{
			uint pos = 1 + nSample / 2;
			uint mod = nSample%2;
			data[pos] = (data[pos] & mask16[mod]) | (value << 16*mod);
			break;
		}
		default: SetField(data+1, ss*8, nSample, value);
	}
//	SetField(data+1, ss*8, nSample, value);
}



 

uint countOnes(K2Tree * k2tree, Node *node, int _x, uint * entries) {
	uint * A = node->data;
	int x = _x;
	if (!x) {
		*entries = 0;
		return 0;
	}

	if (k2tree->inFirstLevels || !k2tree->useDictionary) {
		int ns = getNSamples(k2tree, NODETYPE(node->bid));
		int bitsInicio = ns*(k2tree->inFirstLevels?SAMPLESIZEBITS1:SAMPLESIZEBITS2);
		int haiSamples = ns;
		*entries = x;
		uint ones = 0;
		if (k2tree->inFirstLevels) {
			if (haiSamples) {
				int sample = x/SAMPLEPOSBITS;
//				if (_x == 1025) DEBUG("sample = %d de %d (nt = %d)\n", sample, ns, NODETYPE(node->bid));
				if (sample) {
					ones = getSampleValue(A, sample-1, (k2tree->inFirstLevels?SAMPLESIZE1:SAMPLESIZE2));
					bitsInicio += SAMPLEPOSBITS * (sample);
					x -= SAMPLEPOSBITS * (sample);
				}
			}
			ones += _countOnes2(A+1, bitsInicio, x + bitsInicio);
		}

		return ones;
	} else {
		uint ones2b = 0;
		int bytesInicio = getNSamples(k2tree, NODETYPE(node->bid))*SAMPLESIZE2;
		uint entries2b = countEntries(((byte *)(A+1))+bytesInicio, x/8);
		entries2b *= k2tree->kValues[k2tree->nLevels - 1] * k2tree->kValues[k2tree->nLevels - 1];
		*entries = entries2b;
		return ones2b;
	}
}



//Only in first levels
void alterSamples(K2Tree * k2tree, Node * node, int iniPos, int diffOnes) {
	if (k2tree->inFirstLevels || k2tree->useDictionary) {
		int nSamples = getNSamples(k2tree,NODETYPE(node->bid));
		if (!nSamples) return;
		int nodeSize = GETSIZE(node->data[0]);
		int i;
		int firstSample = iniPos / SAMPLEPOSBITS;
		uint start = firstSample * SAMPLEPOSBITS;

		for (i = firstSample; i < nSamples; i++) {
			if (start >= nodeSize) break;
			setSampleValue(node->data, i, getSampleValue(node->data, i, SAMPLESIZE1) + diffOnes, SAMPLESIZE1);
			start +=SAMPLEPOSBITS;
		}
	}
}

void fillSamples(K2Tree * k2tree, Node * node, int iniPos) {
	if (!ISLEAF(node->data[0])){
		return;
	}
	if (k2tree->inFirstLevels) {
		int nSamples = getNSamples(k2tree,NODETYPE(node->bid));
		if (!nSamples) return;
		int nodeSize = GETSIZE(node->data[0]);
		int i;
		int samplesOffsets = nSamples * SAMPLESIZEBITS1;
		int firstSample = iniPos / SAMPLEPOSBITS;
		uint start = firstSample * SAMPLEPOSBITS;
		int end = start + SAMPLEPOSBITS;
		int ones = 0;

		if (iniPos > 0) {
			uint foo;
			ones = countOnes(k2tree, node, start, &foo);
		}

		for (i = firstSample; i < nSamples; i++) {
			if (start >= nodeSize) break;
			if (end > nodeSize) end = nodeSize;
//			samples acumulativos
			ones += _countOnes2(node->data+1, start + samplesOffsets, end + samplesOffsets);
//			samples individuais
//			ones = _countOnes2(node->data+1, start + samplesOffsets, end + samplesOffsets);
			setSampleValue(node->data, i, ones, SAMPLESIZE1);

			start +=SAMPLEPOSBITS;
			end += SAMPLEPOSBITS;
		}
	} else {
		if (k2tree->useDictionary) {
			int nSamples = getNSamples(k2tree,NODETYPE(node->bid));
			if (!nSamples) return;
			int nodeSize = GETSIZE(node->data[0]);
			int i;
			int samplesOffsets = nSamples * SAMPLESIZE2;
			int firstSample = iniPos / SAMPLEPOSBITS;
			int start = firstSample * SAMPLEPOSBITS;
			int end = start + SAMPLEPOSBITS;
			int entries = 0;
			byte * bdata = ((byte *)(node->data+1)+samplesOffsets);
			for (i = firstSample; i < nSamples; i++) {
				if (start >= nodeSize) break;
				if (end > nodeSize) end = nodeSize;
				entries = countEntries(bdata+start/8, SAMPLEPOS);
				setSampleValue(node->data, i, entries, SAMPLESIZE2);
				start +=SAMPLEPOSBITS;
				end += SAMPLEPOSBITS;
			}
		}
	}
}

int getNodeType(K2Tree * k2tree, int sizebits, int leaf) {
	int i;
	for (i = 0; i < k2tree->mgr->nSizes; i++) {
		int overhead = 32 + (leaf?getNSamples(k2tree, i) * (k2tree->inFirstLevels?SAMPLESIZEBITS1:SAMPLESIZEBITS2) : 0);
		int maxSize = k2tree->mgr->delegates[i]->blockSize;
		if (sizebits + overhead <= maxSize * 8) return i;
	}
	return -1;
}

uint nLocateEntry(K2Tree * k2tree, Node * node, int nEntry) {
	int ns = getNSamples(k2tree, NODETYPE(node->bid));
	int bytesInicio = ns*SAMPLESIZE2;  //Dicionario só no último nivel
	uint acumEntries = 0, entries = 0;

	int i = 0;
	for (i = 0; i < ns; i++) {
		entries = getSampleValue(node->data, i, SAMPLESIZE2);
		if (acumEntries + entries >= nEntry)
			break;
		acumEntries += entries;
	}
	byte * bdata = ((byte *)(node->data+1))+bytesInicio+i*SAMPLEPOS;
	uint ret = locateEntry(bdata, SAMPLEPOS, nEntry - acumEntries);
	return ret + i*SAMPLEPOS;
}

uint * getWordSingleOne(uint pos, uint length) {
	uint i;
	for (i = 0; i < (length * length) / 8; i++)
		PLAYGROUND[i] = 0;
	bitset((uint *)PLAYGROUND, pos);
	return PLAYGROUND;
}

void printWordMatrix(uint * word, uint k) {
	uint i;
	for (i = 0; i < k * k; i++) {
		printf("%d", bitget(word,i)?1:0);
		if (!((i+1)%k)) printf("\n");
	}
}

void printWordEncoded(byte * word, uint nbytes) {
	uint i;
	for (i = 0; i < nbytes; i++) {
		printf("%02x", word[i]);
	}
	printf("\n");
}


static inline void addEntryToInternalNode(uint firstLevels, uint * nodeData, uint nodeSize, uint pos, uint entries, uint ones, BID node) {
	if (firstLevels) {
		memmove(nodeData+1+(pos+1)*3,nodeData+1+pos*3, (nodeSize/ENTRYSIZEBITS1 - pos) * 12);
		nodeData[1 + pos * 3] = entries;
		nodeData[1 + pos * 3 + 1] = ones;
		nodeData[1 + pos * 3 + 2] = node;
		SETSIZE(nodeData[0], GETSIZE(nodeData[0] + ENTRYSIZEBITS1));
	} else {
//		DEBUG("Moving %d bytes from word %d to word %d\n", (nodeSize/ENTRYSIZEBITS2 - pos) * 8, 1+(pos)*2, 1+(pos+1)*2);
		memmove(nodeData+1+(pos+1)*2,nodeData+1+pos*2, (nodeSize/ENTRYSIZEBITS2 - pos) * 8);
		nodeData[1 + pos * 2] = entries;
		nodeData[1 + pos * 2 + 1] = node;
		SETSIZE(nodeData[0], GETSIZE(nodeData[0] + ENTRYSIZEBITS2));
	}
}

static inline void updateEntryInternalNode(uint firstLevels, uint * nodeData, uint pos, uint entries, uint ones, BID node) {
	if (firstLevels) {
		nodeData[1+pos*3]=entries;
		nodeData[1+pos*3+1] = ones;
		nodeData[1+pos*3+2] = node;
	} else {
		nodeData[1+pos*2]=entries;
		nodeData[1+pos*2+1] = node;
	}
}

static inline void updateInternalNodeDiff(uint firstLevels, Node *node, uint offset, int size, int ones) {
	if (firstLevels)  {
		uint * dataAux = (uint *) node->data;
		dataAux[1+offset*3] += size;
		dataAux[1+offset*3+1] += ones;

	} else {
		uint * dataAux = (uint *) node->data;
		dataAux[1+offset*2] += size;
	}
}

static inline void updateInternalNodeDiffSetBid(uint firstLevels, Node *node, uint offset, int sizeAdded, int onesAdded, BID bid) {
	if (firstLevels) {
		uint * dataAux = (uint *) node->data;
		dataAux[1+offset*3] += sizeAdded;
		dataAux[1+offset*3+1] += onesAdded;
		dataAux[1+offset*3+2] = bid;
	} else {
		uint * dataAux = (uint *) node->data;
		dataAux[1+offset*2] += sizeAdded;
		dataAux[1+offset*2+1] = bid;
	}
}


uint findChildAdd1(uint * _data, uint nEntries, ull pos, uint offsetPosReal, ull * _acumOnes, ull * _acumEntries, BID * child, ull *_curEntries) {
	uint i = 0;
	ull curEntries = 0;
	ull acumEntries=0, acumOnes = 0;
	uint * data = _data + 1;
	while(1) {
		curEntries = *data;
		if (acumEntries + curEntries >= pos-offsetPosReal) break;
		acumEntries += curEntries;
		acumOnes 	+= *(data+1);
		data+=3;
		i++;
	}
	*_acumOnes = acumOnes;
	*_acumEntries = acumEntries;
	*child = *(data+2);
	*_curEntries = curEntries;
	return i;
}

uint findChild1(uint * _data, uint nEntries, ull pos, uint offsetPosReal, byte operation, ull * _acumOnes, ull * _acumEntries, BID * child, ull *_curEntries) {
	if (operation == ADD) return findChildAdd1( _data, nEntries, pos, offsetPosReal, _acumOnes, _acumEntries, child, _curEntries);
	else {
		uint i = 0;
		ull curEntries = 0;
		ull acumEntries=0, acumOnes = 0;
		uint * data = _data + 1;
		while(i < nEntries) {
			curEntries = *data;
			if (acumEntries + curEntries > pos) break;
			acumEntries += curEntries;
			acumOnes 	+= *(data+1);
			data+=3;
			i++;
		}
		*_acumOnes = acumOnes;
		*_acumEntries = acumEntries;
		*child = *(data+2);
		*_curEntries = curEntries;
		return i;
	}
}

uint findChildAdd2(uint * _data, uint nEntries, ull pos, uint offsetPosReal, ull * _curEntries, ull * _acumEntries, BID * child) {
	uint i = 0;
	ull curEntries = 0;
	ull acumEntries=0;
	uint * data = _data + 1;
	while(1) {
//		printf("[%d:+%d=%d/%d-%d]", i, *data, acumEntries + *data, pos, offsetPosReal);fflush(stdout);
		curEntries = *data;
		if (acumEntries + curEntries >= pos-offsetPosReal) break;
		acumEntries += curEntries;
		data+=2;
		i++;
	}
	*_curEntries = curEntries;
	*_acumEntries = acumEntries;
	*child = *(data+1);
	return i;
}

uint findChild2(uint * _data, uint nEntries, ull pos, uint offsetPosReal, byte operation, ull * _curEntries, ull * _acumEntries, BID * child) {
	if (operation == ADD) return findChildAdd2(_data, nEntries, pos, offsetPosReal, _curEntries, _acumEntries, child);
	else {

		uint i = 0;
		ull curEntries = 0;
		ull acumEntries=0;
		uint * data = _data + 1;
		while(i < nEntries) {
			curEntries = *data;
			if (acumEntries + curEntries > pos) break;
			acumEntries += curEntries;
			data+=2;
			i++;
		}

		*_acumEntries = acumEntries;
		*_curEntries = curEntries;
		*child = *(data+1);
		return i;
	}
}

uint getValues1(Node * node, ull *entries, ull * ones) {
	uint * nodeData = node->data;
	uint nEntries = (nodeData[0]&SIZE)/ENTRYSIZEBITS1;
	uint i;
	nodeData++;

	*entries = 0; *ones = 0;
	for (i = 0; i < nEntries; i++) {
		*entries+=*nodeData;
		*ones += *(nodeData+1);
		nodeData+=3;
	}
	return nEntries;
}

uint getValues2(Node * node, ull *entries) {
	uint * nodeData = node->data;
	uint nEntries = (nodeData[0]&SIZE)/ENTRYSIZEBITS2;
	uint i;
	nodeData++;

	*entries = 0;
	for (i = 0; i < nEntries; i++) {
		*entries+=*nodeData;
		nodeData+=2;
	}
	return nEntries;
}

uint getValues(uint firstLevels, Node * node, ull * entries, ull * ones) {
	if (firstLevels) return getValues1(node, entries, ones);
	else return getValues2(node, entries);
}




static inline uint getSplitPoint(K2Tree * k2tree, uint size, uint * data, uint sampleBits, ull leafOffset) {
	uint split = size / 2;
	uint curLevel, kk;
	ull curpos = split + leafOffset;
	int isLastLevel = !k2tree->inFirstLevels;
	if (!isLastLevel || ! k2tree->useDictionary) {
		for (curLevel = 0; curLevel < k2tree->nLevels && k2tree->startLevels[curLevel] < curpos; curLevel++);
		curLevel--;
		kk = k2tree->kValues[curLevel]*k2tree->kValues[curLevel];

		uint startOfLevel;
		if (leafOffset < k2tree->startLevels[curLevel]) {
			startOfLevel = (uint) (k2tree->startLevels[curLevel] - leafOffset);
		} else {
			startOfLevel = 0;
		}
		split -= (split - startOfLevel) % kk;
	} else {
		split -= split % 8;
		while (bitget(data, split+sampleBits+7)) {
			split -= 8;
		}
		split +=8;
	}

	return split;
}


void updateAncestors(K2Tree * k2tree, uint depth, Node **path, uint *offsets, int deltaEntries, int deltaOnes, BID bid) {
	int i;
	for (i = 0; i < depth - 1; i++) {
		if (path[i] != NULL) {
			updateInternalNodeDiff(k2tree->inFirstLevels, path[i], offsets[i], deltaEntries, deltaOnes);
			saveNode(k2tree->mgr, path[i]);
		}
		maxPosLevel[i] += deltaEntries;
	}
	if (path[i] != NULL) {
		/*We need this in case we are reallocating the node.*/
		updateInternalNodeDiffSetBid(k2tree->inFirstLevels, path[i], offsets[i], deltaEntries, deltaOnes, bid);
		saveNode(k2tree->mgr, path[i]);
		maxPosLevel[i] += deltaEntries;
	}

}

void updateAncestorsAbs(K2Tree * k2tree, uint depth, Node **path, uint *offsets, uint entries, uint ones, BID bid) {
	if (path[depth] == NULL) return;
	long curEntries = GETNENTRIES(k2tree->inFirstLevels, path[depth], offsets[depth]);
	long curOnes = GETNONES(k2tree->inFirstLevels, path[depth], offsets[depth]);
	updateEntryInternalNode(k2tree->inFirstLevels, path[depth]->data, offsets[depth], entries, ones, bid);
	ull nentries1 = 0, nones1 = 0;
	saveNode(k2tree->mgr, path[depth]);
	if (path[depth-1]) {
		long difEntries = curEntries - (long) entries;
		long difOnes = curOnes - (long) ones;
		nentries1 = GETNENTRIES(k2tree->inFirstLevels, path[depth-1], offsets[depth-1]) - difEntries;
		nones1 = GETNONES(k2tree->inFirstLevels, path[depth-1], offsets[depth-1]) - difOnes;
	} else {
		getValues(k2tree->inFirstLevels, path[depth], &nentries1, &nones1);
	}
	maxPosLevel[depth] = nentries1;

	updateAncestorsAbs(k2tree, depth-1, path, offsets, (uint)nentries1, (uint)nones1, path[depth]->bid);
}

void splitNode(K2Tree * k2tree, uint * data, uint sampleBits, Node **_n1, Node **_n2, uint nt, uint split, uint curSize) {

	ASSERT(split > 0 && split < curSize);
	Node * n1, *n2;

	int ss = k2tree->inFirstLevels ? SAMPLESIZEBITS1 : SAMPLESIZEBITS2;
	n1 = allocateANode(k2tree->mgr, getNodeType(k2tree, 32 + split, nt == LEAF));
	int sampleBits1 = nt == LEAF ?getNSamples(k2tree, NODETYPE(n1->bid))*ss:0;
	bit_memcpy(n1->data+1, sampleBits1, data+1, sampleBits, split, 1);
	n1->data[0] = nt | split;

	n2 = allocateANode(k2tree->mgr, getNodeType(k2tree, 32 + curSize - split, nt == LEAF));
	int sampleBits2 = nt == LEAF?getNSamples(k2tree, NODETYPE(n2->bid))*ss:0;
	bit_memcpy(n2->data+1, sampleBits2, data+1, sampleBits + split, curSize - split, 1);
	n2->data[0] = nt | (curSize - split);

	*_n1 = n1;
	*_n2 = n2;
}

uint splitLeaf(K2Tree *k2tree, Node * node, Node **_n1, Node **_n2, ull leafOffset) {
	uint split;
	uint curSize;
	uint * data = (uint *) node->data;

	curSize = GETSIZE(data[0]);

	int sampleBits = getNSamples(k2tree, NODETYPE(node->bid))*(k2tree->inFirstLevels?SAMPLESIZEBITS1:SAMPLESIZEBITS2);

	split = getSplitPoint(k2tree, curSize, data + 1, sampleBits, leafOffset);

	splitNode(k2tree, data, sampleBits, _n1, _n2, LEAF, split, curSize);

	return split;
}

uint splitInternalNode(K2Tree * k2tree, Node * node, Node **_n1, Node **_n2) {
	Node * reallocated = allocateANode(k2tree->mgr, 0);
	Node * newInternalNode = allocateANode(k2tree->mgr, 0);
	uint * nodeData = node->data;
	uint nodeSize = GETSIZE(nodeData[0]);
	uint split = nodeSize / 2;
	uint * newInternalNodeData = newInternalNode->data;
	uint * reallocatedData = reallocated->data;

	int entrySizeBits = (k2tree->inFirstLevels?ENTRYSIZEBITS1:ENTRYSIZEBITS2);
	split -= split % entrySizeBits;

	SETSIZE(reallocatedData[0], split);
	SETSIZE(newInternalNodeData[0], nodeSize - split);
	memcpy(reallocatedData+1, nodeData+1, split / 8);
//	DEBUG("copying to second leaf from int %d, %d bytes\n", split/32, (nodeSize-split)/8);
	memcpy(newInternalNodeData + 1, nodeData + 1 + (split / 32), (nodeSize - split) /8);
	*_n1 = reallocated;
	*_n2 = newInternalNode;
	return split;
}

void reallocInternalNode(K2Tree * k2tree, Node **node, uint **data, uint nodeType, uint add) {
	int newNodeType = add ? (nodeType+1) : nodeType - 1;
	int sizeToCopy = add ? k2tree->mgr->delegates[nodeType]->blockSize : k2tree->mgr->delegates[newNodeType]->blockSize;
	Node * reallocated = allocateANode(k2tree->mgr, newNodeType);
	uint * reallocatedData = (uint *) reallocated->data;

	memcpy(reallocatedData, *data, sizeToCopy);



	if ((*node)->bid == k2tree->curRootBid) {
		k2tree->curRootBid = reallocated->bid;
		if (k2tree->inFirstLevels)
			k2tree->bidroot1 = k2tree->curRootBid;
		else
			k2tree->bidroot2 = k2tree->curRootBid;
	}
	destroyNode(k2tree->mgr, *node);

	*node = reallocated;
	*data = reallocatedData;
}

void reallocLeaf(K2Tree * k2tree, Node **node, uint **data, uint nodeType, uint add) {
	int newNodeType = add ? (nodeType+1) : nodeType - 1;

	int ss = k2tree->inFirstLevels ? SAMPLESIZE1:SAMPLESIZE2;

	int newSamplesSize = getNSamples(k2tree, newNodeType)*ss;
	int oldSamplesSize = getNSamples(k2tree, nodeType)*ss;

	Node * reallocated = allocateANode(k2tree->mgr, newNodeType);
	uint * reallocatedData = (uint *) reallocated->data;

	int samplesToCopy = add ? oldSamplesSize : newSamplesSize;
	memcpy(reallocatedData+1, *data+1, samplesToCopy);

	reallocatedData[0] = (*data)[0];
	bit_memcpy(reallocatedData+1, newSamplesSize*8, *data+1, oldSamplesSize*8, GETSIZE(*data[0]), 1);

	if ((*node)->bid == k2tree->curRootBid) {
		k2tree->curRootBid = reallocated->bid;
		if (k2tree->inFirstLevels)
			k2tree->bidroot1 = k2tree->curRootBid;
		else
			k2tree->bidroot2 = k2tree->curRootBid;
	}
	destroyNode(k2tree->mgr, *node);

	*node = reallocated;
	*data = reallocatedData;
}

void updateInternalNodeAddEntry(K2Tree *k2tree, Node **path, uint *offsets, uint i,
		ull entries1, ull ones1, BID node1, ull entries2, ull ones2, BID node2,
		uint addEntry, uint increaseOffset) {

//	TRACE("node %x, entries %ld-%ld-%d, %ld-%ld-%d, at offset %d, add=%d, increase=%d\n", path[i], entries1, ones1, node1,
//			entries2, ones2, node2, offsets[i], addEntry, increaseOffset);
//	printNode(k2tree, path[i]);
//	if (path[i-1])
//		if (maxPosLevel[i] != GETNENTRIES(k2tree->inFirstLevels, path[i-1], offsets[i-1]))
//			DEBUG("fails before update!\n");

	Node * internalNode = path[i];
	uint * internalNodeData;
	uint nodeSize, internalNodeType;

	int entrySizeBits = (k2tree->inFirstLevels?ENTRYSIZEBITS1:ENTRYSIZEBITS2);

	if (internalNode == NULL) {
		/*If internal node is NULL we have to add a new one (a new root node)*/
		if (!addEntry) {
			return;
		} else {
			INFO("Adding new root (%d) at level %d\n", k2tree->inFirstLevels, i);
			INFO("values %ld %ld %ld %ld\n", entries1, ones1, entries2, ones2);
			printf("+I");
			fflush(stdout);
			internalNode = allocateANode(k2tree->mgr, 0);
			((uint *) internalNode->data)[0] = INTERNAL | entrySizeBits;
			path[i] = internalNode;
			initPosLevel[i] = 0L;
			acumOnesLevel[i] = 0;
			maxPosLevel[i] = 0xFFFFFFFFFFFFFFL;
			offsets[i] = 0;
			if (k2tree->inFirstLevels)
				k2tree->depth1++;
			else
				k2tree->depth2++;
			setRoot(k2tree, internalNode->bid);
		}
	}

	internalNodeData = (uint *) internalNode->data;
	nodeSize = GETSIZE(internalNodeData[0]);

	long diffEntries1 = (long) entries1 - (long) GETNENTRIES(k2tree->inFirstLevels, internalNode, offsets[i]);
	long diffOnes1 = (long) ones1 - (long) GETNONES(k2tree->inFirstLevels, internalNode, offsets[i]);

	updateEntryInternalNode(k2tree->inFirstLevels, internalNodeData, offsets[i], entries1, ones1, node1);
	internalNodeType = NODETYPE(internalNode->bid);

#ifndef NDEBUG
	if (path[i-1]){
		ull fe, fo;
		getValues(k2tree->inFirstLevels, internalNode, &fe, &fo);
		ASSERT(GETNENTRIES(k2tree->inFirstLevels, path[i-1], offsets[i-1])+diffEntries1 == fe
				&& (!k2tree->inFirstLevels || GETNONES(k2tree->inFirstLevels, path[i-1], offsets[i-1])+diffOnes1 == fo));
	}
#endif

//	printf("j");
	if (addEntry) {
		/*A new entry must be put in this internal node*/
		uint maxInternalNodeSize = k2tree->mgr->delegates[internalNodeType]->blockSize;
		int fits = nodeSize + entrySizeBits + 32 <= maxInternalNodeSize * 8;

		int canExpand = internalNodeType < k2tree->mgr->nSizes-1;
		if (fits || canExpand) {

//			printf("k");
			if (!fits) {
//				printf("Ri");
//				printNode(k2tree, path[i]);
				reallocInternalNode(k2tree, &internalNode, &internalNodeData, internalNodeType, 1);
				path[i] = internalNode;
//				TRACE("Realloc to %x\n", path[i]);
//				printNode(k2tree, path[i]);
			}

			addEntryToInternalNode(k2tree->inFirstLevels, internalNodeData, nodeSize, offsets[i] + 1, entries2, ones2, node2);

//			printNode(k2tree, path[i]);

			if (path[i-1]) {
//				printf("r");fflush(stdout);
				entries1 = ((long) GETNENTRIES(k2tree->inFirstLevels, path[i-1], offsets[i-1])) + diffEntries1 + entries2;
				ones1 = ((long)GETNONES(k2tree->inFirstLevels, path[i-1], offsets[i-1])) + diffOnes1 + ones2;

//#ifndef NDEBUG
//				ull fe, fo;
//				getValues(k2tree->inFirstLevels, internalNode, &fe, &fo);
//				ASSERT(entries1 == fe && (!k2tree->inFirstLevels || ones1 == fo));
//#endif
			} else {
				getValues(k2tree->inFirstLevels, internalNode, &entries1, &ones1);
			}
			maxPosLevel[i] = entries1;
			node1 = path[i]->bid;
			/*In upper levels, we do not need to add new entries, just update*/
			addEntry = 0;
			saveNode(k2tree->mgr, internalNode);
			if (increaseOffset) {
				offsets[i]++;
			}

		} else {
			/*We need to split an internal node										*/

//			printf("Si");
			Node * reallocated;
			Node * newInternalNode;
			uint posToAdd = offsets[i] + 1;

			uint split = splitInternalNode(k2tree, internalNode, &reallocated, &newInternalNode);

			internalNodeData = reallocated->data;
			uint * newInternalNodeData = newInternalNode->data;

			int firstNode = split/entrySizeBits >= posToAdd;

			if (path[i]->bid == k2tree->curRootBid) {
				setRoot(k2tree, firstNode?reallocated->bid:newInternalNode->bid);
			}

			destroyNode(k2tree->mgr, path[i]);
			path[i] = firstNode?reallocated:newInternalNode;

			internalNode = reallocated;

			/*Node is split. Add new entry*/

			if (increaseOffset) {
				offsets[i]++;
			}


			if (firstNode) {
				ASSERT(GETSIZE(internalNodeData[0]) + 32 + entrySizeBits <= k2tree->mgr->delegates[NODETYPE(reallocated->bid)]->blockSize*8,
						"After split any of the nodes should be able to hold a new entry. Block size too small or too many node sizes.\n"
						"The implementation assumes the partition is suitable.");
				addEntryToInternalNode(k2tree->inFirstLevels, internalNodeData, split, posToAdd, entries2, ones2, node2);

				increaseOffset = 0;
			} else {
				ASSERT(GETSIZE(newInternalNodeData[0]) + 32 + entrySizeBits <= k2tree->mgr->delegates[NODETYPE(newInternalNode->bid)]->blockSize*8,
										"After split any of the nodes should be able to hold a new entry. Block size too small or too many node sizes.\n"
										"The implementation assumes the partition is suitable.");
//				printf("2");
//				DEBUG("Adding to second node. split = %d, posToAdd = %d, entrySizeBits = %d\n", split, posToAdd, entrySizeBits);
				addEntryToInternalNode(k2tree->inFirstLevels, newInternalNodeData, nodeSize - split, posToAdd - split/entrySizeBits, entries2, ones2, node2);

//				path[i] = newInternalNode;
				increaseOffset = 1;
				offsets[i] -= split / entrySizeBits;
			}

			getValues(k2tree->inFirstLevels, internalNode, &entries1,&ones1);
			node1 = internalNode->bid;
			if (path[i-1]) {
//				printf("i");
//				fflush(stdout);
				long totalEntries = GETNENTRIES(k2tree->inFirstLevels, path[i-1], offsets[i-1]) + diffEntries1 + entries2;
				long totalOnes = GETNONES(k2tree->inFirstLevels, path[i-1], offsets[i-1]) + diffOnes1 + ones2;
				entries2 = totalEntries - (long)entries1;
				ones2 = totalOnes - (long)ones1;
//#ifndef NDEBUG
//				ull fe, fo;
//				getValues(k2tree->inFirstLevels, newInternalNode, &fe,&fo);
//				ASSERT(entries2 == fe && (!k2tree->inFirstLevels || ones2 == fo));
//#endif
			} else {
				getValues(k2tree->inFirstLevels, newInternalNode, &entries2,&ones2);
			}
			node2 = newInternalNode->bid;

			if (firstNode) {
				maxPosLevel[i]=entries1;
			} else {
				acumOnesLevel[i] += ones1;
				initPosLevel[i]+=entries1;
				maxPosLevel[i] = entries2;
			}

			saveNode(k2tree->mgr, internalNode);
			saveNode(k2tree->mgr, newInternalNode);

			if (path[i] == internalNode) releaseNode(k2tree->mgr, newInternalNode);
			else releaseNode(k2tree->mgr, internalNode);

		}

	} else {

		if(path[i-1]) {
//			printf("x");
			entries1 = ((long) GETNENTRIES(k2tree->inFirstLevels, path[i-1], offsets[i-1])) + diffEntries1;
			ones1 = ((long)GETNONES(k2tree->inFirstLevels, path[i-1], offsets[i-1])) + diffOnes1;
//			ull fe, fo;
//			getValues(k2tree->inFirstLevels, path[i], &fe, &fo);
//			if (entries1 != fe || (k2tree->inFirstLevels && ones1 != fo)) {
//				printNode(k2tree, path[i-1]);
//				printNode(k2tree, path[i]);
//				DEBUG("%ld %ld %ld %ld %ld %ld\n", entries1, ones1, fe, fo, diffEntries1, diffOnes1);
//			}
		} else {
			getValues(k2tree->inFirstLevels, path[i], &entries1, &ones1);
		}
		node1 = path[i]->bid;

		saveNode(k2tree->mgr, path[i]);
		maxPosLevel[i] = entries1;
	}

	updateInternalNodeAddEntry(k2tree, path, offsets, i-1, entries1, ones1, node1, entries2, ones2, node2, addEntry, increaseOffset);
//	TRACE("DONE %x\n", path[i]);
}

void updateInternalNodeRemoveEntry(K2Tree *k2tree, Node **path, uint *offsets, uint i,
		ull entries1, ull ones1, BID node1, ull entries2, ull ones2, BID node2, uint removeEntry, int changeOffset) {

	Node * internalNode = path[i];
	uint * internalNodeData;
	uint nodeSize, internalNodeType,
			maxInternalNodeSize;

	internalNodeData = (uint *) internalNode->data;
	nodeSize = GETSIZE(internalNodeData[0]);

	int myChangeOffset = 0;

	uint entrySizeBits = k2tree->inFirstLevels?ENTRYSIZEBITS1:ENTRYSIZEBITS2;

	updateEntryInternalNode(k2tree->inFirstLevels, internalNodeData, offsets[i], entries1, ones1, node1);
	internalNodeType = NODETYPE(internalNode->bid);
	maxInternalNodeSize = k2tree->mgr->delegates[internalNodeType]->blockSize;

	if (removeEntry) {
		/*A entry must be removed from this internal node*/
//		DEBUG("Removing entry\n");
		int isRoot = internalNode->bid == k2tree->curRootBid;
		int underflow = !isRoot && nodeSize - entrySizeBits < ((maxInternalNodeSize - 4) *8)/2;
		int canShrink = internalNodeType > 0;
		int pos = offsets[i]+1;
		int entrySizeWords = entrySizeBits/32;
//		DEBUG("moving %d %d %d %d %d %d %d\n", NODETYPE(internalNode->bid), pos*entrySizeWords, (pos+1)*entrySizeWords, (nodeSize/entrySizeBits - pos + 1) * entrySizeWords*4, nodeSize, entrySizeBits, pos);
		memmove(internalNodeData+1+pos*entrySizeWords,internalNodeData+1+(pos+1)*entrySizeWords, (nodeSize/entrySizeBits - (pos + 1)) * entrySizeWords*4);
		SETSIZE(internalNodeData[0], GETSIZE(internalNodeData[0]) - entrySizeBits);

		if (isRoot && GETSIZE(internalNodeData[0]) == entrySizeBits)  {
			INFO("Root will disappear!!(%d) New depth = %d\n", k2tree->inFirstLevels, (k2tree->inFirstLevels?k2tree->depth1:k2tree->depth2)-1);
			setRoot(k2tree, path[i+1]->bid);
			if (k2tree->inFirstLevels)	k2tree->depth1--;
			else k2tree->depth2--;
			destroyNode(k2tree->mgr, path[i]);
			path[i] = NULL;
			return;
		}

		int hasSibling = path[i-1] && GETSIZE(path[i-1]->data[0]) > entrySizeBits;

		if (underflow && ! hasSibling) DEBUG("Check this!\n");

		underflow = underflow && hasSibling;

		if (!underflow || canShrink) {

			if (underflow) {
//				INFO("Underflow. We can shrink\n");
				reallocInternalNode(k2tree, &internalNode, &internalNodeData, internalNodeType, 0);
				path[i] = internalNode;
			}

			ull nentries1=0, nones1=0;
			getValues(k2tree->inFirstLevels, internalNode, &nentries1, &nones1);
			maxPosLevel[i] = nentries1;
			node1 = path[i]->bid;

			/*In upper levels, we just update the values*/
			updateAncestorsAbs(k2tree, i-1, path, offsets, (uint)nentries1, (uint)nones1, path[i]->bid);
			saveNode(k2tree->mgr, path[i]);

			return;

		} else {
			/*We need to merge internal nodes										*/

//			INFO("We need to merge internal nodes!\n");

			int leftSibling = 0;
			uint curSize, siblingSize;
			uint curOffset, siblingOffset;

			curSize = GETSIZE(internalNodeData[0]);
			curOffset = offsets[i-1];
			if (curOffset > 0) {
				leftSibling = 1;
				siblingOffset = curOffset - 1;
			} else {
				leftSibling = 0;
				siblingOffset = curOffset + 1;
			}

			Node * siblingNode = readNode(k2tree->mgr, path[i-1]->data[k2tree->inFirstLevels?1+siblingOffset*3+2:1+siblingOffset*2+1]);

			siblingSize = GETSIZE(siblingNode->data[0]);


			uint * firstData = leftSibling?siblingNode->data:path[i]->data;
			uint firstSize = leftSibling?siblingSize:curSize;
			uint * secondData = leftSibling?path[i]->data:siblingNode->data;
			uint secondSize = leftSibling?curSize:siblingSize;
//			uint maxBlockSize = k2tree->mgr->delegates[k2tree->mgr->nSizes-1]->blockSize;

			int newNodeType = getNodeType(k2tree, curSize + siblingSize + 32, 0);
			if (newNodeType != -1) {
				printf("Mi");fflush(stdout);
				//We can put both nodes in a single leaf

				Node * newNode = allocateANode(k2tree->mgr, newNodeType);
				uint * newNodeData = newNode->data;

				memcpy(newNodeData+1, firstData+1, firstSize / 8);
				memcpy(newNodeData+1 + firstSize / 32, secondData+1, secondSize / 8);
				newNodeData[0] = INTERNAL | (firstSize + secondSize);

				ull nentries = 0, nones = 0;
				getValues(k2tree->inFirstLevels, newNode, &nentries, &nones);

				if (leftSibling) offsets[i-1]--;
				destroyNode(k2tree->mgr, path[i]);
				destroyNode(k2tree->mgr, siblingNode);
				path[i] = newNode;
//				printNode(k2tree, path[i]);

				if (leftSibling) {
					initPosLevel[i] -= GETNENTRIES(k2tree->inFirstLevels, path[i-1], curOffset);
					acumOnesLevel[i] -= GETNONES(k2tree->inFirstLevels, path[i-1], curOffset);
//					initPosLevel[i] += entries1;
//					acumOnesLevel[i] += ones1;
				}
				maxPosLevel[i]=entries1;

				updateInternalNodeRemoveEntry(k2tree, path, offsets, i -1, (ull)nentries, (ull)nones, newNode->bid,
						0, 0, 0, 1, 0);


			} else {
//				printf("Rdi");fflush(stdout);
//				printf("%d\n", offsets[i-1]);
//				printNode(k2tree, path[i-1]);
//				printNode(k2tree, path[i]);
//				printNode(k2tree, siblingNode);

				uint * newData = (uint *)malloc(4 + (firstSize + secondSize)/8);
				memcpy(newData+1, firstData+1, firstSize/8);
				memcpy(newData+1 + firstSize / 32 , secondData+1, secondSize / 8);
				newData[0] = INTERNAL | (firstSize + secondSize);

				Node * node1, *node2;
				uint newSize;
				newSize = GETSIZE(newData[0]);
				uint split = (firstSize + secondSize)/2;
				split -= split % entrySizeBits;
				splitNode(k2tree, newData, 0, &node1, &node2, INTERNAL, split, newSize);
				ull entries1 = 0, ones1 = 0, entries2 = 0, ones2 = 0;

				free(newData);

				getValues(k2tree->inFirstLevels, node1, &entries1, &ones1);
				getValues(k2tree->inFirstLevels, node2, &entries2, &ones2);

				saveNode(k2tree->mgr, node1);
				saveNode(k2tree->mgr, node2);
				destroyNode(k2tree->mgr, path[i]);
				destroyNode(k2tree->mgr, siblingNode);

				uint posofdeletion = leftSibling?entrySizeBits*offsets[i]+siblingSize:entrySizeBits*offsets[i];

//				initPosLevel[i] -= path[i-1]->data[k2tree->inFirstLevels?1+curOffset*3:1+siblingOffset*2];

				if (leftSibling)  {
					offsets[i-1]--;
					initPosLevel[i] -= GETNENTRIES(k2tree->inFirstLevels, path[i-1], offsets[i-1]);
					acumOnesLevel[i] -= GETNENTRIES(k2tree->inFirstLevels, path[i-1], offsets[i-1]);
				}
//				printf("%d\n", offsets[i-1]);
//				printNode(k2tree, path[i-1]);
//				printNode(k2tree, node1);
//				printNode(k2tree, node2);

				if (posofdeletion >= split) {
//					printf("2");
					path[i] = node2;
					releaseNode(k2tree->mgr, node1);
					maxPosLevel[i] = entries2;
					initPosLevel[i] += entries1;
					acumOnesLevel[i] += ones1;
//					offsets[i-1]++;
					myChangeOffset = 1;
				} else {
					path[i] = node1;
					maxPosLevel[i] = entries1;
					releaseNode(k2tree->mgr, node2);
				}



				updateInternalNodeRemoveEntry(k2tree, path, offsets, i-1, (ull)entries1, (ull)ones1, node1->bid,
						(ull)entries2, (ull)ones2, node2->bid, 0, myChangeOffset);

//				printf("%d\n", offsets[i-1]);
//				printNode(k2tree, path[i-1]);
//				printNode(k2tree, path[i]);

			}
		}

	} else {
		updateEntryInternalNode(k2tree->inFirstLevels, internalNodeData, offsets[i]+1, entries2, ones2, node2);
		ull nentries1 = 0, nones1 = 0;
//		printNode(path[i]);
		getValues(k2tree->inFirstLevels, path[i], &nentries1, &nones1);
		node1 = path[i]->bid;

		updateAncestorsAbs(k2tree, i-1, path, offsets, (uint)nentries1, (uint)nones1, path[i]->bid);

		saveNode(k2tree->mgr, path[i]);

		offsets[i] += changeOffset;

	}

}

void addToNode(K2Tree * k2tree, Node *node, uint newDataOffset, uint diffsize, uint newDataSize, uint *newData) {
	uint * data = node->data;
	uint curSize = GETSIZE(data[0]);
	int addedSize = 0;
//	if (k2tree->inFirstLevels) addedSize += getNSamples(k2tree, NODETYPE(node->bid))*SAMPLESIZEBITS;
	addedSize += getNSamples(k2tree, NODETYPE(node->bid))*(k2tree->inFirstLevels?SAMPLESIZEBITS1:SAMPLESIZEBITS2);
	makeSpace(data + 1, curSize + addedSize, newDataOffset + addedSize, diffsize);
	SETSIZE(data[0], curSize+diffsize);
	bit_memcpy(data+1, newDataOffset + addedSize, newData, 0, newDataSize, 0);
}


void changeLeaf(K2Tree * k2tree, Node ** path, uint * offsets, uint depth,
		uint * newData, uint newDataSizeBits, uint oldDataSizeBits, uint newDataOffsetBits,
		uint newDataEntries, uint oldDataEntries, uint newDataOnes, uint oldDataOnes,
		ull *leafOffset, ull * ones, ull * pos) {

	int size = newDataSizeBits - oldDataSizeBits;
	int deltaOnes = newDataOnes - oldDataOnes;
	int deltaEntries = newDataEntries - oldDataEntries;

	int sampleSizeBits = k2tree->inFirstLevels?SAMPLESIZEBITS1:SAMPLESIZEBITS2;

	int nodeType = NODETYPE(path[depth]->bid);
	uint * data = path[depth]->data;
	uint curSize = GETSIZE(data[0]);
	uint nSamples = getNSamples(k2tree, nodeType);
	int bitsSamples = nSamples * sampleSizeBits;

	if (newDataSizeBits >= oldDataSizeBits) {

//		printf("m");
		uint maxSizeBlock = k2tree->mgr->delegates[nodeType]->blockSize;

		int overhead = bitsSamples;
		int fits = size + curSize + overhead <= (maxSizeBlock - 4) * 8;
		int canRealloc =  (nodeType < k2tree->mgr->nSizes - 1);
		if (fits || canRealloc) {

			if (!fits)  {
//					DEBUG("Realloc leaf! (%d)\n", k2tree->inFirstLevels);
//				printf("Rl");fflush(stdout);
				reallocLeaf(k2tree, &path[depth], &data, nodeType, 1);
			}

			/*We have space to put the new entry (either we already had or we had to realloc the node)*/
			addToNode(k2tree, path[depth], newDataOffsetBits, size, newDataSizeBits, newData);
			updateAncestors(k2tree, depth, path, offsets, deltaEntries, deltaOnes, path[depth]->bid);
			fillSamples(k2tree, path[depth], newDataOffsetBits);
			maxPosLevel[depth]+=deltaEntries;
		} else {
//			DEBUG("Splitting leaf!(firstlevels=%d)\n", k2tree->inFirstLevels);
//			printf("Sl");fflush(stdout);
			/*We do not have space. We need to split the leaf*/

			Node * reallocated, *newNode;
			//uint * reallocatedData;
			uint *newNodeData;

			uint totalEntries = 0, totalOnes = 0;
			if (path[depth-1]) {
				totalEntries = GETNENTRIES(k2tree->inFirstLevels, path[depth-1], offsets[depth-1])+deltaEntries;
				totalOnes = GETNONES(k2tree->inFirstLevels, path[depth-1], offsets[depth-1])+deltaOnes;
			} else {
//#ifndef NDEBUG
//				if (GETSIZE(path[depth]->data[0] == 7040))
//					printNode(k2tree, path[depth]);
//				totalOnes = countOnes(k2tree, path[depth], GETSIZE(path[depth]->data[0]), &totalEntries);
//				totalOnes += deltaOnes;
//				totalEntries += deltaEntries;
//#endif
			}

			uint split = splitLeaf(k2tree, path[depth],	&reallocated, &newNode, *leafOffset);
			int addToSecondNode;

			//reallocatedData = reallocated->data;
			newNodeData = newNode->data;

			if (path[depth]->bid == k2tree->curRootBid) {
				setRoot(k2tree, reallocated->bid);
			}
			destroyNode(k2tree->mgr, path[depth]);
			path[depth] = reallocated;
			data = path[depth]->data;

			addToSecondNode = newDataOffsetBits >= split;

			/*After node is split, add the new entry.*/

			if (addToSecondNode) {
//				printf("2");
//				DEBUG("Adding to second node\n");
				addToNode(k2tree, newNode, newDataOffsetBits - split, size, newDataSizeBits, newData);
			} else {
				addToNode(k2tree, reallocated, newDataOffsetBits, size, newDataSizeBits, newData);
			}

			saveNode(k2tree->mgr, reallocated);
			saveNode(k2tree->mgr, newNode);

			fillSamples(k2tree, reallocated, 0);
			fillSamples(k2tree, newNode, 0);

			uint _entries1, _entries2;
			ull entries1, entries2;
			ull ones1, ones2;
			BID node1, node2;

			byte addEntry = 1;

			ones1 = countOnes(k2tree, path[depth], GETSIZE(data[0]), &_entries1);
			if (path[depth-1]) {
//#ifdef NDEBUG
				ones2 = totalOnes - ones1;
				_entries2 = totalEntries - _entries1;
//#else
//				ones2 = countOnes(k2tree, newNode,  GETSIZE(newNodeData[0]), &_entries2);
//#endif

			} else {
				ones2 = countOnes(k2tree, newNode,  GETSIZE(newNodeData[0]), &_entries2);
			}

//			ASSERT(!k2tree->inFirstLevels || (ones1 + ones2 == totalOnes));
//			ASSERT(_entries1 + _entries2 == totalEntries);

			entries1 = _entries1;
			entries2 = _entries2;

			node1 = path[depth]->bid;
			node2 = newNode->bid;

			if (addToSecondNode) {
				*ones += ones1;
				*pos = *pos - split;

				if (path[depth]->bid == k2tree->curRootBid) {
					setRoot(k2tree, newNode->bid);
				}

				releaseNode(k2tree->mgr, path[depth]);


				path[depth] = newNode;
				initPosLevel[depth]+=entries1;
				acumOnesLevel[depth]+=ones1;
				maxPosLevel[depth] = entries2;
				*leafOffset -= entries1;
			} else {
				releaseNode(k2tree->mgr, newNode);
				maxPosLevel[depth] = entries1;
			}

			updateInternalNodeAddEntry(k2tree, path, offsets, depth-1,
						entries1, ones1, node1, entries2, ones2, node2, addEntry, addToSecondNode);
		}

	}
	else /*if (newDataSizeBits < oldDataSizeBits) */ {
		deleteSpace(data + 1, curSize + bitsSamples, newDataOffsetBits + bitsSamples, -size);
		SETSIZE(data[0], curSize + size);
		bit_memcpy(data+1, newDataOffsetBits + bitsSamples, newData, 0, newDataSizeBits, 0);


		/*We need to delete kk bits*/
		int deleteNode = path[depth]->bid != k2tree->curRootBid && nodeType == 0 && (GETSIZE(data[0]) + getNSamples(k2tree,0)*sampleSizeBits <= k2tree->mgr->delegates[0]->blockSize*8/2);

		if (path[depth]->bid != k2tree->curRootBid) {
			uint nEntriesParent = GETSIZE(path[depth-1]->data[0]) / (k2tree->inFirstLevels?ENTRYSIZEBITS1: ENTRYSIZEBITS2);
			deleteNode &= nEntriesParent > 1;
		}

		if (!deleteNode) {
			int reduceNode = (nodeType > 0) && (GETSIZE(data[0]) + bitsSamples <= (k2tree->mgr->delegates[nodeType-1]->blockSize-4) * 8);

			if (reduceNode)	 {
//				DEBUG("Reducing leaf (%d)\n", k2tree->inFirstLevels);
//				printf("-l");fflush(stdout);
				reallocLeaf(k2tree, &path[depth], &data, nodeType, 0);
			}
			updateAncestors(k2tree, depth, path, offsets, deltaEntries, deltaOnes, path[depth]->bid);
			fillSamples(k2tree, path[depth], newDataOffsetBits);
			maxPosLevel[depth]+=deltaEntries;
		} else {

			int leftSibling = 0;
			uint curSize, siblingSize;
			uint curOffset, siblingOffset;

			curSize = GETSIZE(data[0]);

			curOffset = offsets[depth-1];
			if (curOffset > 0) {
				leftSibling = 1;
				siblingOffset = curOffset - 1;
			} else {
				leftSibling = 0;
				siblingOffset = curOffset + 1;
			}

			uint siblingEntries = GETNENTRIES(k2tree->inFirstLevels, path[depth-1],siblingOffset);
			uint siblingOnes = GETNONES(k2tree->inFirstLevels, path[depth-1],siblingOffset);
			Node * siblingNode = readNode(k2tree->mgr, path[depth-1]->data[k2tree->inFirstLevels?(1+siblingOffset*3+2):(1+siblingOffset*2+1)]);
			siblingSize = GETSIZE(siblingNode->data[0]);


			uint * firstData = leftSibling?siblingNode->data:path[depth]->data;
			uint firstSize = leftSibling?siblingSize:curSize;
			uint * secondData = leftSibling?path[depth]->data:siblingNode->data;
			uint secondSize = leftSibling?curSize:siblingSize;

//				int curOverhead = k2tree->inFirstLevels ? getNSamples(k2tree, NODETYPE(path[depth]->bid))*SAMPLESIZEBITS:0;
//				int siblingOverhead = k2tree->inFirstLevels ? getNSamples(k2tree, NODETYPE(siblingNode->bid))*SAMPLESIZEBITS:0;
			int curOverhead = getNSamples(k2tree, NODETYPE(path[depth]->bid))*sampleSizeBits;
			int siblingOverhead = getNSamples(k2tree, NODETYPE(siblingNode->bid))*sampleSizeBits;


			int firstOverhead = leftSibling ? siblingOverhead : curOverhead;
			int secondOverhead = leftSibling ? curOverhead : siblingOverhead;

			int newNodeType = getNodeType(k2tree, 32 + curSize + siblingSize, 1);
			if (newNodeType != -1) {
//				printf("Ml");fflush(stdout);
//				DEBUG("Merging two leaves (%d)\n", k2tree->inFirstLevels);

				//We can put both nodes in a single leaf
				Node * newNode = allocateANode(k2tree->mgr, newNodeType);
				uint * newNodeData = newNode->data;

//					int newNodeOverhead = k2tree->inFirstLevels ? getNSamples(k2tree, NODETYPE(newNode->bid))*SAMPLESIZEBITS : 0;
				int newNodeOverhead = getNSamples(k2tree, NODETYPE(newNode->bid))*sampleSizeBits;

				bit_memcpy(newNodeData+1, newNodeOverhead, firstData+1, firstOverhead, firstSize, 1);
				bit_memcpy(newNodeData+1, newNodeOverhead + firstSize, secondData+1, secondOverhead, secondSize, 1);
				newNodeData[0] = LEAF | (firstSize + secondSize);

				fillSamples(k2tree, newNode, 0);

				uint entries1, ones1;

//				*pos = leftSibling ? (*pos + siblingSize) : *pos;
				*pos = leftSibling ? (*pos + siblingEntries) : *pos;

				ones1 = countOnes(k2tree, newNode, firstSize + secondSize, &entries1);

				destroyNode(k2tree->mgr, path[depth]);
				destroyNode(k2tree->mgr, siblingNode);
				path[depth] = newNode;
				if (leftSibling) offsets[depth-1]--;
				if (leftSibling) {
					initPosLevel[depth] -= siblingEntries;
					acumOnesLevel[depth] -= siblingOnes;
				}
				maxPosLevel[depth] = entries1;
				updateInternalNodeRemoveEntry(k2tree, path, offsets, depth -1, (ull)entries1, (ull)ones1, newNode->bid,
						0, 0, 0, 1, 0);


			} else {
//				printf("Rdl");fflush(stdout);
//					DEBUG("Redistributing content between two leaves (%d)\n", k2tree->inFirstLevels);
				uint * newData = (uint *)malloc((1 + (firstSize + secondSize + 31)/32)*sizeof(uint));

				bit_memcpy(newData+1, 0, firstData+1, firstOverhead, firstSize, 1);
				bit_memcpy(newData+1, firstSize, secondData+1, secondOverhead, secondSize, 1);
				newData[0] = LEAF | (firstSize + secondSize);

				{
					Node * node1, *node2;
					uint newSize;
					newSize = GETSIZE(newData[0]);


					uint split = getSplitPoint(k2tree, newSize, newData + 1, 0, leftSibling?*leafOffset-siblingEntries:*leafOffset);
//						printNode(path[depth-1]);
//						DEBUG("%d\n",k2tree->mgr->delegates[0]->cachePositions[14]);
					splitNode(k2tree, newData, 0, &node1, &node2, LEAF, split, newSize);
					uint entries1, ones1, entries2, ones2;

					fillSamples(k2tree, node1, 0);
					fillSamples(k2tree, node2, 0);

					free(newData);
//						DEBUG("split = %d\n", split);

					ones1 = countOnes(k2tree, node1, split, &entries1);
					ones2 = countOnes(k2tree, node2, newSize - split, &entries2);

//					if (leftSibling) offsets[depth-1]--;

					saveNode(k2tree->mgr, node1);
					saveNode(k2tree->mgr, node2);
					destroyNode(k2tree->mgr, path[depth]);
					destroyNode(k2tree->mgr, siblingNode);

					uint posofdeletion = leftSibling?newDataOffsetBits+siblingSize:newDataOffsetBits;
					if (leftSibling) {
						initPosLevel[depth] -= siblingEntries;
						acumOnesLevel[depth] -= siblingOnes;
					}

					if (leftSibling) offsets[depth-1]--;

					int mychangeoffset = 0;
					if (posofdeletion > split) {
//						printf("2");
						path[depth] = node2;
						releaseNode(k2tree->mgr, node1);
						initPosLevel[depth] += entries1;
						acumOnesLevel[depth] += ones1;
						maxPosLevel[depth] = entries2;
						mychangeoffset = 1;
					} else {
						path[depth] = node1;
						releaseNode(k2tree->mgr, node2);
						maxPosLevel[depth] = entries1;

					}


//						printNode(path[depth-1]);
					updateInternalNodeRemoveEntry(k2tree, path, offsets, depth-1, (ull)entries1, (ull)ones1, node1->bid,
								(ull)entries2, (ull)ones2, node2->bid, 0, mychangeoffset);

				}

			}

		}
		saveNode(k2tree->mgr, path[depth]);

	}


}


uint ENTRIESARRAY[6];



void resetPath(K2Tree * k2tree) {
	int i;
//	printf("resetting path\n");
	for (i = MAX_LEVELS - 1; path[i]; i--) {
		releaseNode(k2tree->mgr, path[i]);
		path[i] = NULL;
		initPosLevel[i] = 0;
		acumOnesLevel[i] = 0;
		offsets[i] = 0;
	}
	//	saveNode(k2tree->mgr, path[0]);
	//	depth = 0;
	//	memset(path, 0, MAX_LEVELS*sizeof(Node *));
	//	k2tree->curRoot = k2tree->inFirstLevels? k2tree->root1:k2tree->root2;
	depth = MAX_LEVELS - (k2tree->inFirstLevels ? k2tree->depth1
			: k2tree->depth2);
	path[depth] = readNode(k2tree->mgr, k2tree->inFirstLevels ? k2tree->bidroot1 : k2tree->bidroot2);
	k2tree->curRootBid = path[depth]->bid;

	maxPosLevel[depth] = 0xFFFFFFFFFFFF;
}


Node * goToLeaf(K2Tree * k2tree, uint operation, uint reset, ull pos, ull comparepos, uint offsetPosReal) {
	//ull origPos = pos;
	if (reset)
		resetPath(k2tree);


	uint levelsUp = 0;
	if (!reset) {

		while (path[depth-1] && (comparepos >= initPosLevel[depth] + maxPosLevel[depth] || comparepos < initPosLevel[depth])) {
			releaseNode(k2tree->mgr, path[depth]);
			depth--;
			levelsUp++;
		}
		pos -= initPosLevel[depth];
	} else {
		levelsUp = (k2tree->inFirstLevels?k2tree->depth1:k2tree->depth2)-1;
	}

	//Find the leaf
	int iter;
	int entrySizeBits = k2tree->inFirstLevels?ENTRYSIZEBITS1:ENTRYSIZEBITS2;
	{
		ull acumOnes, acumEntries;
		uint i, nEntries;
		BID child;

		for (iter = 0; iter < levelsUp; iter++) {

			uint * data = (uint *) path[depth]->data;

			uint curSize = GETSIZE(data[0]);
			/*Internal node: find the child*/

			acumOnes = 0;
			acumEntries = 0;
			nEntries = curSize / entrySizeBits;

			if (k2tree->inFirstLevels) {
				i = findChild1(data, nEntries, pos, offsetPosReal, operation, &acumOnes, &acumEntries, &child, &maxPosLevel[depth+1]);
			} else {
				i = findChild2(data, nEntries, pos, offsetPosReal, operation, &maxPosLevel[depth+1], &acumEntries, &child);
			}
			initPosLevel[depth+1] = initPosLevel[depth]+acumEntries;
			acumOnesLevel[depth+1] = acumOnesLevel[depth]+acumOnes;

			offsets[depth] = i;
			depth ++;
			pos -= acumEntries;

			path[depth] = readNode(k2tree->mgr, child);

//#ifndef NDEBUG
//			if (path[depth-1]) {
//				ull thisEntries =0L;
//				ull thisOnes = 0L;
//				if (iter == levelsUp - 1) {
//					//thisOnes = countOnes(k2tree, path[depth], GETSIZE(path[depth]->data[0]), &thisEntries);
//					thisOnes = countOnes(k2tree, path[depth], GETSIZE(path[depth]->data[0]), (uint*) &thisEntries);
//				} else {
//					getValues(k2tree->inFirstLevels, path[depth], &thisEntries, &thisOnes);
//				}
//				ASSERT(thisEntries == GETNENTRIES(k2tree->inFirstLevels, path[depth-1], offsets[depth-1]));
//				ASSERT(thisOnes == GETNONES(k2tree->inFirstLevels, path[depth-1], offsets[depth-1]));
//			}
//#endif

		}
	}



	return path[depth];

}


/*****
 * Rebuild the tree if vocabulary compression is too bad
 */



Node * getChild(K2Tree * k2tree, Node * internalNode, int child) {
	if (k2tree->inFirstLevels) {
		BID childBid = (internalNode->data[1+child*3+2]);
		return readNode(k2tree->mgr, childBid);
	} else {
		BID childBid = (internalNode->data[1+child*2+1]);
		return readNode(k2tree->mgr, childBid);
	}

}

void * doSomething(K2Tree * k2tree, void (*firstfun)(K2Tree * k2tree), void (*funInternalNodes)(K2Tree *k2tree, Node * internalNode),
		void (*funLeaves)(K2Tree * k2tree, Node * leaf), void * (*lastfun)(K2Tree * k2tree)) {
	Node * path[MAX_LEVELS];
	uint offsets[MAX_LEVELS];
	uint nchildren[MAX_LEVELS];
	uint depth;
	int i;

	if (firstfun) {
		firstfun(k2tree);
	}

	depth = MAX_LEVELS - (k2tree->inFirstLevels?k2tree->depth1:k2tree->depth2);

	for (i = 0; i < MAX_LEVELS; i++) {
		path[i] = NULL;
		offsets[i] = 0;
	}
	path[depth] = readNode(k2tree->mgr, k2tree->inFirstLevels?k2tree->bidroot1:k2tree->bidroot2);
	k2tree->curRootBid = path[depth]->bid;
	offsets[depth] = 0;
	nchildren[depth]=GETNENTRIES2(k2tree, path[depth]);

	while(path[depth]) {
		if (!ISLEAF(path[depth]->data[0])) {
			if (offsets[depth] < nchildren[depth]) {
				//Navigate to next child
				path[depth+1] = getChild(k2tree, path[depth], offsets[depth]++);
				nchildren[depth+1] = GETNENTRIES2(k2tree, path[depth+1]);
				offsets[depth+1] = 0;
				depth++;
			} else {
				if (funInternalNodes) (*funInternalNodes)(k2tree, path[depth]);
				releaseNode(k2tree->mgr, path[depth]);
				depth--;
			}
		} else {
			if (funLeaves) (*funLeaves)(k2tree, path[depth]);
			releaseNode(k2tree->mgr, path[depth]);
			depth--;
		}

	}

	if (lastfun) return (*lastfun)(k2tree);
	else return NULL;
}


void printNodeData(K2Tree * k2tree, BID bid, uint * data) {
	int i;
	if (ISLEAF(data[0])) {
//		int bitsSamples = k2tree->inFirstLevels ? getNSamples(k2tree, NODETYPE(bid))* SAMPLESIZEBITS : 0;
		int bitsSamples = getNSamples(k2tree, NODETYPE(bid))* (k2tree->inFirstLevels?SAMPLESIZEBITS1:SAMPLESIZEBITS2);
		printf("LEAF: (bid= %d, size = %d, samples = %d bits)\n", bid, GETSIZE(data[0]), bitsSamples);
		for (i = 0; i*32 < GETSIZE(data[0]) + bitsSamples; i++) {
			printf("%08x ", data[1+i]);
		}
		printf("\n");
		fflush(stdout);
	} else {
		int entrySizeBits = k2tree->inFirstLevels?ENTRYSIZEBITS1:ENTRYSIZEBITS2;
		if (k2tree->inFirstLevels) {
			printf("INTERNAL: (bid =%d, size=%d)\n", bid, GETSIZE(data[0]));
			for (i = 0; i < GETSIZE(data[0]) / entrySizeBits; i++) {
				printf(" (%04d-%04d-%04d)  ", data[1+i*3], data[1+i*3+1], data[1+i*3+2]);
			}
			printf("\n");
		} else {
			printf("INTERNAL: (bid =%d, size=%d)\n", bid, GETSIZE(data[0]));
			for (i = 0; i < GETSIZE(data[0]) / entrySizeBits; i++) {
				printf(" (%04d-%04d)  ", data[1+i*2], data[1+i*2+1]);
			}
			printf("\n");
		}
	}
}

void printNode(K2Tree * k2tree, Node * node) {
	if (node == NULL) {
		DEBUG("<NULL>\n");
		return;
	}
	printNodeData(k2tree, node->bid, node->data);
}
