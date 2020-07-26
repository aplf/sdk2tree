#include "k2tree-common.h"
#include "k2tree-impl.h"
#include "encoding.h"

//extern Node * path[MAX_LEVELS];
//extern uint offsets[MAX_LEVELS];
//extern ull initPosLevel[MAX_LEVELS];
//extern ull maxPosLevel[MAX_LEVELS];
//extern uint acumOnesLevel[MAX_LEVELS];
//extern uint depth;

uint getPosition(K2Tree * k2tree, ull *pos, byte operation, uint k2treelevel, uint reset) {
//	fflush(stdout);
	unsigned char * oldData = NULL;
	uint *newData = (uint *) k2tree->PLAYGROUND[0];
	uint newDataSizeBits, oldDataSizeBits, newDataOffsetBits, newDataEntries = 0, oldDataEntries = 0, newDataOnes, oldDataOnes;
	uint haveNewData = 0;
	uint i, k, kk, res = 0, curSize = 0;
	uint * data, *dataAux;
	ull ones = 0;
	NodeType nodeType;

	uint offsetPosReal;
	ull leafOffset = 0;

	uint isLastLevel = (k2treelevel == k2tree->nLevels - 1);
	uint useDictionary = isLastLevel && k2tree->useDictionary;

	k2tree->inFirstLevels = !isLastLevel;

	/* These are used in last level, to access vbyte encoded entries */
	uint entryOffset = 0, entryLength = 0;
	uint entry;

	k = k2tree->kValues[k2treelevel];
	kk = k * k;

	if (isLastLevel) *pos-=k2tree->startLevels[k2tree->nLevels-1];

//	printf("%d %d %ld\n", k2tree->bidroot1, k2tree->bidroot2, *pos);

//	ull origPos = *pos;

	offsetPosReal = *pos % kk;
	ull comparepos = *pos;
	if (operation == ADD) comparepos -= offsetPosReal + 1;

	goToLeaf(k2tree, operation, reset, *pos, comparepos, offsetPosReal);

	int sampleSizeBits = k2tree->inFirstLevels?SAMPLESIZEBITS1:SAMPLESIZEBITS2;


	leafOffset = initPosLevel[depth];
	ones = acumOnesLevel[depth];
	*pos -= leafOffset;

	//We are at the leaf now

	data = (uint *) path[depth]->data;
	curSize = GETSIZE(data[0]);
	nodeType = NODETYPE(path[depth] -> bid);

//	int bitsSamples = k2tree->inFirstLevels?getNSamples(k2tree, nodeType) * SAMPLESIZEBITS:0;
	int bitsSamples = getNSamples(k2tree, nodeType) * sampleSizeBits;
	int bytesSamples = bitsSamples/8;

	haveNewData = 0;

	if (!useDictionary) {
		if (operation != ADD) {
			if (*pos >= curSize) return 0;

			res = bitget(data+1,*pos+bitsSamples);

		}
	} else { /* In the last level we have now compressed leaves */
		uint nEntry;
		byte * mybdata = ((byte *)(data+1))+bytesSamples;

		nEntry = *pos / kk;

		entryOffset = nLocateEntry(k2tree, path[depth], nEntry);

		if (operation != ADD) {
			entry = decodeValue(mybdata + entryOffset, &entryLength);
			oldData = (unsigned char *)getWordVocabulary(k2tree->voc, (uint) entry);
			res = bitget((uint *)oldData, *pos % kk);
		}

	}

	switch(operation) {
		case QUERY	:
			break;
		case QUERY_REMOVE :
			if (res) {
				if (!useDictionary) {
					ull realPos = *pos - offsetPosReal;
					int j, allCleared = 1;
					uint *bmdata = data + 1;
					for (j = 0; j < kk; j++) {
						if (realPos + j != *pos && bitget(bmdata, bitsSamples + realPos + j)) {
							allCleared = 0;
							break;
						}
					}
					if (allCleared) res = res | ALL_CLEARED;
				} else {
					int j, allCleared = 1;
					for (j = 0; j < kk; j++) {
						if ((j != ((*pos)%kk)) && bitget((uint *)oldData, j)) {
							allCleared = 0;
							break;
						}
					}
					if (allCleared) res = res | ALL_CLEARED;
				}
			}
			break;
		case CLEAR :
			if (!useDictionary) {
				if (res) {
					bitclean(data+1, bitsSamples + *pos);
					alterSamples(k2tree, path[depth], *pos, -1);
//					fillSamples(k2tree, path[depth], *pos);
					/*If already updated, save node and update tree*/
					saveNode(k2tree->mgr, path[depth]);
					for (i = 0; i < depth; i++) {
						if (path[i] != NULL) {
							dataAux = (uint *) path[i]->data;
							if (k2tree->inFirstLevels)
								dataAux[1 + offsets[i] * 3 + 1]--;
							saveNode(k2tree->mgr, path[i]);
						}
					}
				}

			} else {

				removeEntryVocabulary(k2tree->voc, (byte *) oldData);

				memcpy(newData, oldData, kk/8);
				bitclean(newData, *pos % kk);

				ull value = addEntryVocabulary(k2tree->voc, (unsigned char *)newData);

				{
					uint newEntryLength = getEncodedSize(value);

					haveNewData = 1;
					oldDataSizeBits = entryLength * 8;
					newDataSizeBits = newEntryLength * 8;
					newDataOnes = _countOnes(newData, kk);
					oldDataOnes = newDataOnes + 1;
					newDataEntries = kk;
					oldDataEntries = kk;
					newDataOffsetBits = entryOffset * 8;

					encodeValue(value, (byte *)newData);
				}
			}
			break;
		case SET	:
		{
			if (useDictionary) {

				removeEntryVocabulary(k2tree->voc, (byte *) oldData);

				memcpy(newData, oldData, kk/8);
				bitset(newData, *pos % kk);

				ull value = addEntryVocabulary(k2tree->voc, (unsigned char *)newData);

				{
					uint newEntryLength = getEncodedSize(value);

					haveNewData = 1;
					oldDataSizeBits = entryLength * 8;
					newDataSizeBits = newEntryLength * 8;
					newDataOnes = _countOnes(newData, kk);
					oldDataOnes = newDataOnes - 1;
					newDataEntries = kk;
					oldDataEntries = kk;
					newDataOffsetBits = entryOffset * 8;

					encodeValue(value, (byte *)newData);
				}
			} else {

				if (!res) {
					bitset(data+1, bitsSamples + *pos);

					alterSamples(k2tree, path[depth], *pos, 1);
//					fillSamples(k2tree, path[depth], *pos);

					saveNode(k2tree->mgr, path[depth]);
					for (i = 0; i < depth; i++) {
						if (path[i] != NULL) {
							dataAux = (uint *) path[i]->data;
							if (k2tree->inFirstLevels)
								dataAux[1 + offsets[i] * 3 + 1]++;
							saveNode(k2tree->mgr, path[i]);
						}
					}
				}
			}

			break;
			/*END OF SET*/
		}
		case ADD	:
			{
				haveNewData = 1;
				oldDataSizeBits = 0;
				oldDataOnes = 0;
				newDataOnes = 1;
				newDataEntries = kk;
				oldDataEntries = 0;
				if (useDictionary) {
					unsigned int * word = getWordSingleOne(offsetPosReal, k);
					uint value = addEntryVocabulary(k2tree->voc, (unsigned char *)word);
					newDataSizeBits = getEncodedSize(value) * 8;
					encodeValue(value, (byte *)newData);
					newDataOffsetBits = entryOffset * 8;

				}
				else {
					newDataSizeBits = kk;
					newDataOnes = 1;
					newDataOffsetBits = *pos - offsetPosReal;
					memset(newData, 0, 256);
					bitset((uint *)newData, *pos - newDataOffsetBits);
				}


			/*END OF ADD operation*/

				break;
		}
		case REMOVE :
		{
//						TRA0("Add\n");
				haveNewData = 1;
				newDataSizeBits = 0;
				oldDataOnes = 1;
				newDataOnes = 0;
				newDataEntries = 0;
				oldDataEntries = kk;
				if (useDictionary) {
					oldDataSizeBits = entryLength * 8;
					newDataOffsetBits = entryOffset * 8;
					removeEntryVocabulary(k2tree->voc, (byte *) oldData);
				}
				else {
					oldDataSizeBits = kk;
					newDataOffsetBits = *pos - offsetPosReal;
				}
		}

	} /*END OF switch operation*/


	//Add new content to leaf (increase or decrease leaf size, and split or merge if needed)
	if (haveNewData) {
//		printf("(%d)", newDataSizeBits, oldDataSizeBits);
//		printf("[%d]",(int)newDataEntries-(int)oldDataEntries);
//		DEBUG("size = %d %d\n", newDataSizeBits, oldDataSizeBits);
		changeLeaf(k2tree, path, offsets, depth, newData, newDataSizeBits, oldDataSizeBits, newDataOffsetBits,
				newDataEntries, oldDataEntries, newDataOnes, oldDataOnes, &leafOffset, &ones, pos);
	}

	if (!isLastLevel && operation != REMOVE && operation != CLEAR) {
		uint foo;
		ones += countOnes(k2tree, path[depth], *pos, &foo);
		*pos = ones;
	}

	if (operation != QUERY) {
		saveNode(k2tree->mgr, path[depth]);
	}
	/*We return in *pos the next position to search*/

//	for (i = 0; i < MAX_LEVELS; i++) {
//		if (path[i] != NULL && path[i]!= k2tree->curRoot) {
//			releaseNode(k2tree->mgr, path[i]);
//		}
//	}

//#ifndef NDEBUG
//	{
//		int it = 0;
//		for (it = 0; it < MAX_LEVELS; it++) {
//			if (path[it-1]) {
//				ull thisEntries =0L;
//				ull thisOnes = 0L;
//				if (it == MAX_LEVELS - 1) {
//					thisOnes = countOnes(k2tree, path[it], GETSIZE(path[it]->data[0]), (uint*) &thisEntries);
//					//thisOnes = countOnes(k2tree, path[it], GETSIZE(path[it]->data[0]), &thisEntries);
//				} else {
//					getValues(k2tree->inFirstLevels, path[it], &thisEntries, &thisOnes);
//				}
//				ASSERT(thisEntries == GETNENTRIES(k2tree->inFirstLevels, path[it-1], offsets[it-1]));
//				ASSERT(thisOnes == GETNONES(k2tree->inFirstLevels, path[it-1], offsets[it-1]));
//			}
//		}
//	}
//#endif
	return res;

}


#define SWAPARRAY(a1, a2) {void * aux = a1; a1 = a2; a2 = aux;}

uint doGetNeighbors(K2Tree * k2tree, uint k2treelevel,
		uint *children, uint * offsetOnes, ull *pos, int cleared,
		uint direct, uint rowOrColumn) {

	uint res = 0;

	uint * data;
	ull c_ones = 0;

	uint i,neighborsFound = 0;
	uint k = k2tree->kValues[k2treelevel];
	uint kk = k * k;

	uint isLastLevel = k2treelevel == k2tree->nLevels-1;
	k2tree->inFirstLevels = !isLastLevel;
	//ull leafOffset = 0;

	if (isLastLevel) {
		*pos -= k2tree->startLevels[k2treelevel];
	}

	goToLeaf(k2tree, QUERY, (k2treelevel == 0 || isLastLevel) && !cleared, *pos, *pos, *pos%kk);
	c_ones = acumOnesLevel[depth];
	*pos -= initPosLevel[depth];

	//leafOffset = initPosLevel[depth];
	data = (uint *) path[depth]->data;
	int nodeType = NODETYPE(path[depth]->bid);

	int sampleSizeBits = k2tree->inFirstLevels?SAMPLESIZEBITS1:SAMPLESIZEBITS2;
	int bitsSamples = getNSamples(k2tree, nodeType) * sampleSizeBits;
	int bytesSamples = bitsSamples / 8;

	if (*pos > 0 && !isLastLevel) {
		uint foo;
		c_ones += countOnes(k2tree, path[depth], *pos, &foo);
	}



	if (direct) {
		uint innerOnes = 0;
		if (!isLastLevel || !k2tree->useDictionary) {
//						if (*pos >= GETSIZE(data[0])) {res = 0; goto fin_bucle;}
			for (i = 0; i < k*(rowOrColumn+1); i++) {
				if(bitget(data+1,*pos+bitsSamples+i)) {
					if (i >= k*rowOrColumn) {
						children[neighborsFound] = i % k;
						offsetOnes[neighborsFound] = innerOnes;
						neighborsFound++;
					}
					/*To keep the offset (in ones) for each child*/
					innerOnes++;
				}

			}
		} else {
			uint nEntry = (*pos) / kk;
			uint entryOffset, entry, entryLength;
			byte * bdata = ((byte *)(data+1))+bytesSamples;

			entryOffset = nLocateEntry(k2tree, path[depth], nEntry);
			entry = decodeValue(bdata + entryOffset, &entryLength);

			byte *oldData = (byte *) getWordVocabulary(k2tree->voc, (uint) entry);
			if (rowOrColumn)
				innerOnes = _countOnes((uint *)oldData, k*rowOrColumn);
			for (i = k*rowOrColumn; i < k*(rowOrColumn+1); i++) {
				if(bitget((uint *)oldData, i)) {
					children[neighborsFound] = i % k;
					offsetOnes[neighborsFound] = innerOnes;
					neighborsFound++;
					/*To keep the offset (in ones) for each child*/
					innerOnes++;
				}

			}

		}
	} else  { //nodeFrom == -1, reverse neighbors search
		uint innerOnes = 0;
		if (!isLastLevel || !k2tree->useDictionary) {
//						if (*pos >= GETSIZE(data[0])) return 0;
			uint innerOnes = 0;
			for (i = 0; i < kk; i++) {
				if(bitget(data+1,*pos+bitsSamples+i)) {
					if ((i % k) == rowOrColumn) {
						children[neighborsFound] = i / k;
						offsetOnes[neighborsFound] = innerOnes;
						neighborsFound++;
					}
					innerOnes++;
				}
			}
		} else {
			uint nEntry = (*pos) / kk;
			uint entryOffset, entry, entryLength;
			byte * bdata = ((byte *)(data+1))+bytesSamples;

			entryOffset = nLocateEntry(k2tree, path[depth], nEntry);
			entry = decodeValue(bdata + entryOffset, &entryLength);

			byte *oldData = (byte *) getWordVocabulary(k2tree->voc, (uint) entry);
			for (i = 0; i < kk; i++) {
				if(bitget((uint *)oldData,i)) {
					if ((i % k) == rowOrColumn) {
						children[neighborsFound] = i / k;
						offsetOnes[neighborsFound] = innerOnes;
						neighborsFound++;
					}
					innerOnes++;
				}
			}
		}

	}
	/*We store in *pos the next position to look at*/
	*pos = c_ones;
	res += neighborsFound;
	return res;

}

uint * getNeighbors(K2Tree * k2tree, uint nodeFrom, uint nodeTo) {

//	uint * neighbors = k2tree->lastColumns;
	uint curParallel = 0;
	ull * positions = k2tree->positions[2*curParallel];
	uint * columns = k2tree->columns[2*curParallel] + 1;
	uint * ones = k2tree->ones[2*curParallel];
	ull * lastPositions = k2tree->positions[2*curParallel+1];
	uint * lastColumns = k2tree->columns[2*curParallel+1]+1;
	uint * lastOnes = k2tree->ones[2*curParallel+1];
//	uint * order = globalorder[2*curParallel+1];

	uint nPositions = 0;
	uint nLastPositions = 1;
	int level, j, k;
	ull nextPos = 0;
	//ull oldNextPos;
	uint res;
	uint offsetRowOrColumn = nodeFrom == -1?nodeTo:nodeFrom;
	uint rowOrColumn;
	lastPositions[0] = 0;
	lastColumns[0] = 0;

	k2tree->inFirstLevels = 1;

	for (level = 0; level < k2tree->nLevels ; level++) {
//		printf("nlevel %d got %d\n", level, nLastPositions);
//		if (level < 6)
//			printrow2(nLastPositions, lastColumns);
		int cleared = 0;
		uint kk = k2tree->kValues[level] * k2tree->kValues[level];
		rowOrColumn = offsetRowOrColumn / k2tree->divLevels[level];
		for (j = 0; j < nLastPositions; j++) {
			nextPos = lastPositions[j];
			if (level > 0) {
				nextPos = (nextPos - k2tree->onesBeforeLevel[level-1]) * kk + k2tree-> startLevels[level];
			}
			//oldNextPos = nextPos;

			res = doGetNeighbors(k2tree, level, columns+nPositions, ones+nPositions, &nextPos, cleared, nodeTo==-1U, rowOrColumn);
			cleared = 1;

			for (k = 0; k < res; k++) {
				positions[nPositions + k] = (nextPos + ones[nPositions + k]);
				columns[nPositions+k] = lastColumns[j] + k2tree->divLevels[level] * columns[nPositions + k];
			}

			nPositions += res;
//			printf("got %d\n", res);
		}
		SWAPARRAY(lastPositions, positions);
		SWAPARRAY(lastColumns, columns);
		SWAPARRAY(lastOnes, ones);

		nLastPositions = nPositions;
		nPositions = 0;
		offsetRowOrColumn = offsetRowOrColumn % k2tree->divLevels[level];
	}

	uint * neighbors = lastColumns - 1;
	//neighbors is lastColumns - 1. We just add the number of elements
	neighbors[0] = nLastPositions;

	return neighbors;


}


uint *range[2];
uint doGetRange(K2Tree * k2tree, uint k2treelevel, uint cleared,
		uint *newrows, uint * newcolumns, ull * pos, ull *ret_c_ones,
		uint * offsetOnes, uint * _inirow, uint * _endrow, uint * _inicol, uint * _endcol,
		uint lir, uint ler, uint lic, uint lec) {

//	printf("dgr: %p %d %d %lu %lu %u %u %u %u\n", k2tree, k2treelevel, cleared, *pos, *ret_c_ones,
//			lir, ler, lic, lec);

	uint level = k2treelevel;

	uint * data;
	uint res = 0;
	ull c_ones;
	uint i, j, neighborsFound = 0;
	uint k = k2tree->kValues[k2treelevel];
	uint kk = k * k;
	uint isLastLevel = k2treelevel == k2tree->nLevels-1;
	//ull leafOffset = 0;
	k2tree->inFirstLevels = !isLastLevel;

	if (isLastLevel) {
		*pos -= k2tree->startLevels[k2treelevel];
	}

	goToLeaf(k2tree, QUERY, (k2treelevel == 0 || isLastLevel) && !cleared, *pos, *pos, *pos % kk);
//	goToLeaf(k2tree, QUERY, 1, *pos, *pos, *pos % kk);

	c_ones = acumOnesLevel[depth];

	*pos -= initPosLevel[depth];

	//leafOffset = initPosLevel[depth];

	//We are at the leaf now
	data = (uint *) path[depth]->data;
	int nodeType = NODETYPE(path[depth]->bid);

	int sampleSizeBits = k2tree->inFirstLevels?SAMPLESIZEBITS1:SAMPLESIZEBITS2;
	int bitsSamples = getNSamples(k2tree, nodeType) * sampleSizeBits;
	int bytesSamples = bitsSamples / 8;

	if (*pos > 0 && !isLastLevel) {
		uint foo;
		c_ones += countOnes(k2tree, path[depth], *pos, &foo);
	}

	uint innerOnes = 0;

	if (!isLastLevel || !k2tree->useDictionary) {
	//						if (*pos >= GETSIZE(data[0])) {res = 0; goto fin_bucle;}
	//					innerOnes = countOnes(k2tree, path[depth], firstFrom * i + j);
		for (i = 0; i < k; i++) {
			for (j = 0; j < k; j++) {
				if(bitget(data+1,*pos+bitsSamples+i*k+j)) {
					if (i >= lir / k2tree->divLevels[level]
							  && i <= ler / k2tree->divLevels[level]
							  && j >= lic / k2tree->divLevels[level]
							  && j <= lec / k2tree->divLevels[level]) {
	//									printf("OK! >> [%d %d]\n", i, j);
	//									printf("---[%d %d]\n", i, j);
						newrows[neighborsFound] = i;
						newcolumns[neighborsFound] = j;
						offsetOnes[neighborsFound] = innerOnes;
						_inirow[neighborsFound] = i == lir / k2tree->divLevels[level] ? lir % k2tree->divLevels[level] : 0;
						_endrow[neighborsFound] = i == ler / k2tree->divLevels[level] ? ler % k2tree->divLevels[level] : k2tree->divLevels[level]-1;
						_inicol[neighborsFound] = j == lic / k2tree->divLevels[level] ? lic % k2tree->divLevels[level] : 0;
						_endcol[neighborsFound] = j == lec / k2tree->divLevels[level] ? lec % k2tree->divLevels[level] : k2tree->divLevels[level]-1;
						neighborsFound++;
					}
					/*To keep the offset (in ones) for each child*/
					innerOnes++;
				}
			}

		}
	} else {
		uint nEntry = (*pos) / kk;
		uint entryOffset, entry, entryLength;
		byte * bdata = ((byte *)(data+1))+bytesSamples;

		entryOffset = nLocateEntry(k2tree, path[depth], nEntry);
		entry = decodeValue(bdata + entryOffset, &entryLength);

		byte *oldData = (byte *) getWordVocabulary(k2tree->voc, (uint) entry);

		for (i = 0; i < k; i++) {
			for (j = 0; j < k; j++) {
				if(bitget((uint *)oldData, i*k+j)) {
					if (i>= lir && i <= ler && j >= lic && j <= lec) {
	//									printf("[%d %d]\n", i, j);
						newrows[neighborsFound] = i;
						newcolumns[neighborsFound] = j;
						offsetOnes[neighborsFound] = innerOnes;
						neighborsFound++;
					}
						/*To keep the offset (in ones) for each child*/
					innerOnes++;
				}
			}

		}

	}
	res += neighborsFound;
	*ret_c_ones = c_ones;
	return res;

}



uint *range[2];
uint ** getRange(K2Tree * k2tree, uint leftFrom, uint rightFrom, uint leftTo, uint rightTo) {

//	uint * neighbors = k2tree->lastColumns;
	uint curParallel = 0;
	ull * positions = k2tree->positions[2*curParallel];
	uint * rows = k2tree->rows[2*curParallel] + 1;
	uint * columns = k2tree->columns[2*curParallel] + 1;
	uint * ones = k2tree->ones [2*curParallel];
	ull * lastPositions = k2tree->positions[2*curParallel+1];
	uint * lastRows = k2tree->rows[2*curParallel+1]+1;
	uint * lastColumns = k2tree->columns[2*curParallel+1]+1;
	uint * lastOnes = k2tree->ones[2*curParallel+1];

	uint * inirow = k2tree->firstFrom[2*curParallel];
	uint * endrow = k2tree->lastFrom[2*curParallel];
	uint * inicol = k2tree->firstTo[2*curParallel];
	uint * endcol = k2tree->lastTo[2*curParallel];

	uint * lastinirow = k2tree->firstFrom[2*curParallel+1];
	uint * lastendrow = k2tree->lastFrom[2*curParallel+1];
	uint * lastinicol = k2tree->firstTo[2*curParallel+1];
	uint * lastendcol = k2tree->lastTo[2*curParallel+1];

	uint nPositions = 0;
	uint nLastPositions = 1;
	int level, k, p;
	ull nextPos = 0;
	//ull oldNextPos;
	uint res;

	lastPositions[0] = 0;
	lastRows[0] = 0;
	lastColumns[0] = 0;
	lastinirow[0] = leftFrom;
	lastendrow[0] = rightFrom;
	lastinicol[0] = leftTo;
	lastendcol[0] = rightTo;

	for (level = 0; level < k2tree->nLevels ; level++) {
		if (nLastPositions == 0) break;
		int cleared = 0;
		k2tree->inFirstLevels = level != k2tree->nLevels-1;
		uint kk = k2tree->kValues[level] * k2tree->kValues[level];

//		DEBUG("level %d. Got %d positions\n", level, nLastPositions);
//		if (nLastPositions) {
//			int iter;
//			printf("lastcolumns: ");
//			for (iter = 0; iter < nLastPositions; iter++) {
//				printf("(%d %d) ", lastRows[iter], lastColumns[iter]);
//			}
//			printf("\n");
////			printrow2(nLastPositions, lastColumns[0]);
//		}
		for (p = 0; p < nLastPositions; p++) {

			nextPos = lastPositions[p];
//			printf("pos %d, offset %d in level\n", nextPos, nextPos - k2tree->onesBeforeLevel[level-1]);
			if (level > 0) {
				nextPos = (nextPos - k2tree->onesBeforeLevel[level-1]) * kk + k2tree-> startLevels[level];
			}
			//oldNextPos = nextPos;
//			printf("pos %d\n", nextPos);

			ull c_ones = 0;

//			printf("pos %lu [%d %d] (%d %d %d %d)\n", nextPos,
//					lastRows[p], lastColumns[p], lastinirow[p], lastendrow[p], lastinicol[p], lastendcol[p]);
			res = doGetRange(k2tree, level, cleared,
					rows+nPositions, columns + nPositions, &nextPos, &c_ones, ones + nPositions,
					inirow + nPositions, endrow + nPositions, inicol + nPositions, endcol + nPositions,
					lastinirow[p], lastendrow[p], lastinicol[p], lastendcol[p]);
//			printf("got %d res, %lu %d %d\n", res, nextPos, c_ones, ones[nPositions], ones[nPositions+1]);
			cleared = 1;
//			{

			for (k = 0; k < res; k++) {
				positions[nPositions + k] = (c_ones + ones[nPositions + k]);
				rows[nPositions+k] = lastRows[p] + k2tree->divLevels[level] * rows[nPositions + k];
				columns[nPositions+k] = lastColumns[p] + k2tree->divLevels[level] * columns[nPositions + k];
//				DEBUG("next: %d %d %d %d %d %d %d\n", rows[nPositions+k], columns[nPositions+k], nextPos, ones[nPositions+k]);
			}
			nPositions += res;

		}
		SWAPARRAY(lastPositions, positions);
		SWAPARRAY(lastRows, rows);
		SWAPARRAY(lastColumns, columns);
		SWAPARRAY(lastOnes, ones);
		SWAPARRAY(inirow, lastinirow);
		SWAPARRAY(endrow, lastendrow);
		SWAPARRAY(inicol, lastinicol);
		SWAPARRAY(endcol, lastendcol);

		nLastPositions = nPositions;
		nPositions = 0;

	}


	range[0] = lastRows -1;
	range[1] = lastColumns -1;

	range[0][0] = nLastPositions;

//	{
//		int iter;
//		for (iter = 0; iter < nLastPositions; iter++) {
//			printf("$%d,%d$ ", range[0][iter+1], range[1][iter+1]);
//		}
//		printf("%lu\n", k2tree->nEdges);
//	}

	return range;


}


/*****
 * Rebuild the tree if vocabulary compression is too bad
 */


int contentSize = 0;
int blockSize = 0;
int globalfreqs[MAX_WORDS_VOC];
int ninternalnodes = 0;
int nleaves = 0;
int usedbids[16][4096];

void initInfo(K2Tree * k2tree) {
//	int i;
//	for (i = 0; i < MAX_WORDS_VOC; i++) {
//		globalfreqs[i] = 0;
//	}
	contentSize = 0;
	blockSize = 0;
	ninternalnodes = 0;
	nleaves = 0;

}

void countInternalNode(K2Tree * k2tree, Node * node) {
	int nt = NODETYPE(node->bid);
//	int bid = RAWBID(node->bid);
//	contentSize += GETSIZE(node->data[0]);
	blockSize += k2tree->mgr->delegates[nt]->blockSize;
//	if (bid >= 4096) exit(1/0);
//	usedbids[nt][bid] = 1;
	ninternalnodes++;
}

void updateInfo(K2Tree * k2tree, Node * node) {

	contentSize += GETSIZE(node->data[0]);
	int nodeType = NODETYPE(node->bid);
//	int bid = RAWBID(node->bid);
//	if (bid >= 4096) exit(1/0);
//	usedbids[nodeType][bid] = 1;
	blockSize += k2tree->mgr->delegates[nodeType]->blockSize;

//	int leafSize = GETSIZE(node->data[0]) / 8;
////	int nodeType = NODETYPE(node->bid);
//	int bytesSamples = getNSamples(k2tree, nodeType) * (k2tree->inFirstLevels?SAMPLESIZE1:SAMPLESIZE2);
//
//	int offset = 0;
//	byte * bdata = ((byte *)(node->data+1))+bytesSamples;
//	int leafentries = 0;
//	while (offset < leafSize) {
//		uint size;
//		uint word = decodeValue(bdata+offset, &size);
//		offset += size;
//		globalfreqs[word]++;
//		leafentries++;
//	}
//	DEBUG("leaf size %d bytes (has %d voc words - %d entries)\n", GETSIZE(node->data[0])/8, leafentries, leafentries*64);
	nleaves++;
}

void * showInfo(K2Tree * k2tree) {
//	int i;
//	DEBUG("total size stored = %d bits\n", totalSize);
//	DEBUG("total size used = %d bytes (%d bits) in %d blocks\n", blockSize, blockSize*8, ninternalnodes + nleaves);
//	DEBUG("has %d internal nodes, %d leaves\n", ninternalnodes, nleaves);
//	for (i = 0; i < k2tree->voc->nWords; i++) {
//		if (k2tree->voc->freqs[i] != globalfreqs[i])
//			exit(EXIT_UNEXPECTED);
//	}
	return NULL;
}

void displayTreeInfo(K2Tree * k2tree) {
//	int i, j;

	k2tree->inFirstLevels = 1;
	doSomething(k2tree, initInfo, countInternalNode, updateInfo, showInfo);

	int totalCSize = contentSize;
	int totalBSize = blockSize;
	int totalNodes = ninternalnodes + nleaves;
//			DEBUG("total size used = %d bytes (%d bits) in %d blocks\n", blockSize, blockSize*8, ninternalnodes + nleaves);

	k2tree->inFirstLevels = 0;
	doSomething(k2tree, initInfo, countInternalNode, updateInfo, showInfo);

	totalCSize += contentSize;
	totalBSize += blockSize;
	totalNodes += ninternalnodes + nleaves;

	DEBUG("Content: %08d bytes. Tree: %08d bytes in %06d nodes\n", totalCSize/8, totalBSize, totalNodes);

}

void updateInternalNodes(K2Tree * k2tree, Node ** path, uint depth, uint curentries, BID bid,
		uint * offsets, uint * cursize, uint * entries, uint * maxsize, int packRemaining) {
	int updatenext = 0;
	uint nextentries = 0;
	BID nextbid = 0;

//	printf("updating internal nodes\n");
	//Pack leaf and add entry to upper levels
	if (!path[depth]) {
		if (packRemaining) return;

		path[depth] = allocateANode(k2tree->mgr, 0);
		offsets[depth] = 4;
		maxsize[depth] = k2tree->mgr->delegates[NODETYPE(path[depth]->bid)]->blockSize * 0.9;
		entries[depth] = 0;
		cursize[depth] = 0;
	}
//	DEBUG("adding to internal node(%d)\n", depth);
	if (offsets[depth] + 8 > maxsize[depth]) {
		updatenext = 1;
//		DEBUG("replacing internal node\n");
		nextentries = entries[depth];
		nextbid = path[depth]->bid;

		path[depth]->data[0] = INTERNAL | cursize[depth];
		saveNode(k2tree->mgr, path[depth]);
		releaseNode(k2tree->mgr, path[depth]);

		path[depth] = allocateANode(k2tree->mgr, 0);
		offsets[depth] = 4;
		maxsize[depth] = k2tree->mgr->delegates[NODETYPE(path[depth]->bid)]->blockSize;
		entries[depth] = 0;
		cursize[depth] = 0;
	}
	path[depth]->data[offsets[depth]/4] = curentries;
	path[depth]->data[offsets[depth]/4+1] = bid;
	offsets[depth]+=8;
	cursize[depth]+=64;
	entries[depth]+= curentries;

	if (updatenext)
		updateInternalNodes(k2tree, path, depth-1, nextentries, nextbid, offsets, cursize, entries, maxsize, 0);

	if (packRemaining) {
		path[depth]->data[0] = INTERNAL | cursize[depth];
		saveNode(k2tree->mgr, path[depth]);
		nextentries = entries[depth];
		nextbid = path[depth]->bid;
		updateInternalNodes(k2tree, path, depth-1, nextentries, nextbid, offsets, cursize, entries, maxsize, packRemaining);
	}
}


void rebuildLastLevel(K2Tree * k2tree) {
	Node * oldpath[MAX_LEVELS];
	uint oldoffsets[MAX_LEVELS];
	uint nchildren[MAX_LEVELS];
	Node * newpath[MAX_LEVELS];
	uint newoffsets[MAX_LEVELS];
	uint cursize[MAX_LEVELS];
	uint entries[MAX_LEVELS];
	uint maxsize[MAX_LEVELS];
	uint olddepth, newdepth;
	int i;
	uint oldpos = 0, newpos = 0;

	if (k2tree->inFirstLevels) {
		INFO("k2tree  was set to first levels. Resetting\n");
		k2tree->inFirstLevels = 0;
	}

//	resetPath(k2tree);
	for (i = MAX_LEVELS - 1; path[i]; i--) {
		releaseNode(k2tree->mgr, path[i]);
		path[i] = NULL;
	}

	olddepth = MAX_LEVELS - k2tree->depth2;
	newdepth = MAX_LEVELS - 1;

	for (i = 0; i < MAX_LEVELS; i++) {
		oldpath[i] = NULL;
		oldoffsets[i] = 0;
		newpath[i] = NULL;
		newoffsets[i] = 0;
		entries[i] = 0;
	}
	oldpath[olddepth] = readNode(k2tree->mgr, k2tree->bidroot2);
	oldoffsets[olddepth] = 0;
	nchildren[olddepth]=GETNENTRIES2(k2tree, oldpath[olddepth]);

	newpath[newdepth] = allocateAnyNode(k2tree->mgr);
	newpath[newdepth]->data[0] = LEAF;
	newoffsets[newdepth] = 4 + getNSamples(k2tree, NODETYPE(newpath[newdepth]->bid)) * SAMPLESIZE2;
	maxsize[newdepth] = k2tree->mgr->delegates[NODETYPE(newpath[newdepth]->bid)]->blockSize*0.9;
	cursize[newdepth] = 0;

	//int nleaves = 0;

	while(oldpath[olddepth]) {
//		printf("{%d:%d}\n", oldpath[olddepth]->bid, oldpath[olddepth]->data[0]);
		if (!ISLEAF(oldpath[olddepth]->data[0])) {
//			if (oldoffsets[olddepth] == 0) {
//				printf("[+%d]",oldpath[olddepth]->bid);fflush(stdout);
//			}
			if (oldoffsets[olddepth] < nchildren[olddepth]) {
				//Navigate to next child
				oldpath[olddepth+1] = getChild(k2tree, oldpath[olddepth], oldoffsets[olddepth]++);
				nchildren[olddepth+1] = GETNENTRIES2(k2tree, oldpath[olddepth+1]);
				oldoffsets[olddepth+1] = 0;
				olddepth++;
			} else {
//				printf("[-%d]",oldpath[olddepth]->bid);fflush(stdout);
				//Destroy node (we are rebuilding) ang go back
				destroyNode(k2tree->mgr, oldpath[olddepth]);
				olddepth--;
			}
		} else {
//			printf("<%d>",oldpath[olddepth]->bid);fflush(stdout);
			int leafSize = GETSIZE(oldpath[olddepth]->data[0]) / 8;
			int nodeType = NODETYPE(oldpath[olddepth]->bid);
			int bytesSamples = getNSamples(k2tree, nodeType) * SAMPLESIZE2;

			int offset = 0;
			byte * bdata = ((byte *)(oldpath[olddepth]->data+1))+bytesSamples;
			byte * newbdata = ((byte *)(newpath[newdepth])->data);

//			printNode(k2tree, oldpath[olddepth]);

			while (offset < leafSize) {

				uint size;
				uint word = decodeValue(bdata+offset, &size);

//				INFO("offset %d [max %d], got word %d, freq %d, optimal code %d\n", offset, leafSize, word, vector_get(k2tree->voc->freqs,word), vector_get(k2tree->voc->o2s, word));

				offset += size;
				oldpos +=64;
				uint newword = getOptimalPosition(k2tree->voc, word);



				if (newoffsets[newdepth] + getEncodedSize(newword) > maxsize[newdepth]) {
					newpath[newdepth]->data[0] = LEAF | cursize[newdepth];
					fillSamples(k2tree, newpath[newdepth], 0);

					int curentries = entries[newdepth];
					int bid = newpath[newdepth]->bid;

					saveNode(k2tree->mgr, newpath[newdepth]);
					releaseNode(k2tree->mgr, newpath[newdepth]);

					updateInternalNodes(k2tree, newpath, newdepth-1, curentries, bid, newoffsets, cursize, entries, maxsize, 0);

					newpath[newdepth] = allocateAnyNode(k2tree->mgr);
					newoffsets[newdepth] = 4 + getNSamples(k2tree, NODETYPE(newpath[newdepth]->bid)) * SAMPLESIZE2;
					maxsize[newdepth] = k2tree->mgr->delegates[NODETYPE(newpath[newdepth]->bid)]->blockSize * 0.9;
					cursize[newdepth] = 0;
					entries[newdepth] = 0;

					newbdata = ((byte *)(newpath[newdepth])->data);

				}

				int wb = encodeValue(newword, newbdata + newoffsets[newdepth]);
				newoffsets[newdepth] += wb;
				cursize[newdepth] += wb*8;
				entries[newdepth] += k2tree->voc->wordLength*8;
				newpos += 64;

			}
			//Destroy old node
			destroyNode(k2tree->mgr, oldpath[olddepth]);
//
			//And go up one level
			olddepth--;
		}

	}

	//Pack the remaining nodes
	newdepth = MAX_LEVELS - 1;

	newpath[newdepth]->data[0] = LEAF | cursize[newdepth];
	fillSamples(k2tree, newpath[newdepth], 0);

	int curentries = entries[newdepth];
	int bid = newpath[newdepth]->bid;

	saveNode(k2tree->mgr, newpath[newdepth]);

	updateInternalNodes(k2tree, newpath, newdepth-1, curentries, bid, newoffsets, cursize, entries, maxsize, 1);

	for (i = MAX_LEVELS-1; i >= 0; i--) {
		saveNode(k2tree->mgr, newpath[i]);
		if (i==0||!newpath[i-1]) {
			k2tree->bidroot2 = newpath[i]->bid;
			releaseNode(k2tree->mgr, newpath[i]);
			k2tree->depth2 = MAX_LEVELS - i;
			break;
		}
		releaseNode(k2tree->mgr, newpath[i]);
	}
}

