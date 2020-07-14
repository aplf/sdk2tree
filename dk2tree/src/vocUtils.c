#include "vocUtils.h"

int byteSize(int pos) {
	if (pos < 128) return 1;
	if (pos < 16384) return 2;
	if (pos < 2097152) return 3;
	return 4;
}

VOC * createEmptyVocabulary(uint wordLen) {
	uint maxSizeVoc = DEF_VECTOR_SIZE;

	VOC * voc = (VOC *) malloc(sizeof(VOC));
	voc->maxSize = maxSizeVoc;
	voc->posInTH = vector_init();
	voc->o2s = vector_init();
	voc->s2o = vector_init();
	voc->top = vector_init();
//	voc->posInTH = (uint *) malloc(maxSizeVoc * sizeof(uint));
//	voc->o2s = (uint *) malloc(maxSizeVoc * sizeof(uint));
//	voc->s2o = (uint *) malloc(maxSizeVoc * sizeof(uint));
//	voc->top = (uint *) malloc(MAX_FREQS_VOC * sizeof(uint));

	voc->readonly = 0;
	voc->wordLength = wordLen;
	voc->nWords = 0;
	voc->nWordsReal = 0;
	voc->compressedSize = 0L;
	voc->optimalSize = 0L;

	hash_table t = initialize_hash(voc->maxSize/*FIXME*/, voc->wordLength);
	voc->inverse = t;

	voc->freqs = vector_init();
//	voc->freqs = (uint *) malloc(maxSizeVoc * sizeof(uint));

//	memset(voc->freqs, 0, maxSizeVoc * sizeof(uint));
	voc->maxFreq = 1;
	vector_set(voc->top, 0, 0);
	vector_set(voc->top, 1, 0);

	initStack(&(voc->emptyCodes), DEF_SMALL_SIZE);

	return voc;
}


//void resizeVoc(VOC *voc, uint newSize) {
//	voc->maxSize = newSize;
//	voc->posInTH = realloc(voc->posInTH, voc->maxSize * sizeof(uint));
//	voc->o2s = realloc(voc->o2s, voc->maxSize * sizeof(uint));
//	voc->s2o = realloc(voc->s2o, voc->maxSize * sizeof(uint));
//	voc->freqs = realloc(voc->freqs, voc->maxSize * sizeof(uint));
//
//	MemoryManager mgr = createMemoryManager();
//	hash_table t2 = initialize_hash(newSize, voc->inverse->keySize, mgr);
//	int i;
//	for (i = 0; i < voc->inverse->hashSize; i++) {
//
//	}
//
//}

vector * globalfreqs;

int compareAux(void * p1, void * p2) {
	uint e1 = *((uint *)p1);
	uint e2 = *((uint *)p2);
	return vector_get(globalfreqs,e2)-vector_get(globalfreqs, e1);
}

VOC * loadVocabulary(char * filename, uint readOnly) {

	uint nWords, nWordsReal, wordLen;
//	uint maxSizeVoc = MAX_WORDS_VOC;

	FILE * fv = fopen(filename, "r+");
	VOC * voc;

	DEBUG("Loading vocabulary: %s\n", filename);

	FREAD(&nWords, sizeof(uint), 1, fv); /*reads the number of words of the vocabulary*/
	FREAD(&nWordsReal, sizeof(uint), 1, fv); /*reads the number of words of the vocabulary*/
	FREAD(&wordLen, sizeof(uint), 1, fv);


	voc = (VOC *) malloc(sizeof(VOC));
	voc->readonly = readOnly;
//	voc->words = words;
	voc->wordLength = wordLen;
	voc->nWords = nWords;
	voc->nWordsReal = nWordsReal;
	voc->inverse = NULL;
	voc->freqs = NULL;
	voc->compressedSize = 0L;
	voc->optimalSize = 0L;
	voc->s2o = NULL;
	voc->o2s = NULL;
	voc->top = NULL;

	uint ssizevoc = voc->readonly?1 << bits(nWords):nWords;



	if (!voc->readonly) {
		voc->posInTH = vector_init2(ssizevoc);
		voc->freqs = vector_init2(ssizevoc);
	}


//	MemoryManager mgr = createMemoryManager();

	unsigned char * words = (unsigned char *) malloc(ssizevoc * sizeof(unsigned char) * voc->wordLength);
	FREAD(words, sizeof(char) * voc->wordLength, nWords, fv);

	int i;
	if (voc->readonly) {
		voc->words = words;
	} else {
		hash_table t = initialize_hash(ssizevoc, voc->wordLength);
		voc->inverse = t;

		for (i = 0; i < voc->nWords; i++) {
			uint addr;
			unsigned char * word = words + i * voc->wordLength;
			search(t, word, &addr);
			insertElement(voc->inverse, word, &addr, i);
			if (!voc->readonly)
				vector_set(voc->posInTH, i, addr);
		}
		free(words);
	}

//	memset(voc->freqs, 0, maxSizeVoc * sizeof(uint));

	if (!voc->readonly) {
		uint val;
		for (i = 0; i < nWords; i++) {
			FREAD(&val, sizeof(uint), 1, fv);
			vector_set(voc->freqs, i, val);
		}
	}
//	FREAD(voc->freqs, sizeof(uint), nWords, fv);


	if (!voc->readonly) {
		for (i = 0; i < voc->nWords; i++) {
			voc->compressedSize += byteSize(i)*vector_get(voc->freqs,i);
		}
	}

	if (!voc->readonly) {
		voc->s2o = vector_init2(ssizevoc);
		voc->o2s = vector_init2(ssizevoc);
		voc->top = vector_init2(ssizevoc);

		//Prepare for sorting
		globalfreqs = voc->freqs;
		for (i = 0; i < voc->nWordsReal; i++) {
			vector_set(voc->s2o, i, i);
		}

		vector_sort(voc->s2o, voc->nWordsReal, (__compar_fn_t) compareAux);
	//	qsort(voc->s2o, voc->nWordsReal, sizeof(uint), (__compar_fn_t) compareAux);

		for (i = 0; i < voc-> nWords; i++) {
			vector_set(voc->o2s,vector_get(voc->s2o,i), i);
		}

		for (i = 0; i < voc->nWordsReal; i++) {
			voc->optimalSize += byteSize(i)*vector_get(voc->freqs, vector_get(voc->s2o,i));
		}

		vector_set(voc->top, 0, voc->nWordsReal);
		int oldFreq = 0;
		int oldPos = 0;
		int j;
		for (i = voc->nWordsReal-1; i >= 0; i--) {
			int newFreq = vector_get(voc->freqs, vector_get(voc->o2s, i));
			if (newFreq != oldFreq) {
				for (j = oldFreq + 1; j < newFreq; j++)
					vector_set(voc->top, j, oldPos);
				vector_set(voc->top, newFreq, i);
				oldPos = i;
			}
		}
		TRACE("Voc has %d words\n", voc->nWords);

		//Read empty codes
		initStack(&(voc->emptyCodes), DEF_SMALL_SIZE);
		while(1) {
			uint curCode;
			int readres = fread(&curCode, sizeof(uint), 1 , fv);
			if (readres != 1) break;
			pushStack(&(voc->emptyCodes), curCode);
		}

	}

	fclose(fv);

	return voc;
//	return NULL;
}

void freeVocabulary(VOC * voc) {

	if (!voc->readonly) {
		vector_destroy(voc->posInTH);
		vector_destroy(voc->o2s);
		vector_destroy(voc->s2o);
		vector_destroy(voc->top);


		vector_destroy(voc->freqs);

		freeStack(voc->emptyCodes);
		freeHashTable(voc->inverse, 1);
	} else {
		free(voc->words);
	}

	free(voc);
}

void saveVocabulary(VOC * voc, char * filename) {
	if (voc->readonly) return;
	FILE * fv = openFile(filename);
	fseek(fv, 0, SEEK_SET);
	fwrite(&(voc->nWords), sizeof(uint), 1, fv); /*stores the number of words of the vocabulary*/
	fwrite(&(voc->nWordsReal), sizeof(uint), 1, fv); /*stores the number of words of the vocabulary*/
	fwrite(&(voc->wordLength), sizeof(uint), 1, fv);

	int i;
	for (i = 0; i < voc->nWords; i++) {
//		fwrite(voc->inverse->table[vector_get(voc->posInTH,i)].key, voc->wordLength*sizeof(char), 1, fv);
		fwrite(svector_get(voc->inverse->words, i), voc->wordLength*sizeof(char), 1, fv);
	}

	if (voc->freqs) {
		for (i = 0; i  < voc->nWords; i++) {
			uint val = vector_get(voc->freqs, i);
			fwrite(&val, sizeof(uint), 1, fv);
		}
	}
	while(!isEmptyStack(voc->emptyCodes)) {
		int pos = popStack(&(voc->emptyCodes));
		fwrite(&pos,sizeof(int), 1, fv);
	}

	fclose(fv);
}

unsigned char * getWordVocabulary(VOC * voc, uint word) {
//	return voc->inverse->table[vector_get(voc->posInTH, word)].key;
	if (voc->readonly)
		return voc->words+word*voc->wordLength;
	else
		return svector_get(voc->inverse->words, word);
}

void printWordVocabulary(VOC * voc, uint nword) {
	uint i;
	uint n = 8;
	unsigned char * word = getWordVocabulary(voc, nword);
	for (i = 0; i < n * n; i++) {
		printf("%d", bitgetchar(word, i));
		if (!((i+1)%n)) printf("\n");
	}

}

uint memUsageVocabulary(VOC * voc) {
	uint size = 0;
	size += sizeof(voc);
	size += vector_size(voc->posInTH) * sizeof(uint);
	DEBUG("voc: %d\n", size);
	size += vector_size(voc->freqs) * sizeof(uint);
	DEBUG("freqs: %ld\n", vector_size(voc->freqs) * sizeof(uint));
//	size += (vector_size(voc->s2o) + vector_size(voc->o2s) + vector_size(voc->top)) * sizeof(uint);
	INFO("Optimal voc: (%d+%d+%d) * 4\n", vector_size(voc->s2o), vector_size(voc->o2s), vector_size(voc->top));
	if (voc->inverse)
		size += memUsageHt(voc->inverse, 1);
	return size;
}

uint diskUsageVocabulary(VOC * voc) {
	return 2 * sizeof(uint) + voc->nWords * voc->wordLength * sizeof(char);
}

hash_entry getEntryWord(VOC * voc, unsigned char * word, unsigned int * addr) {
	return search(voc->inverse, word, addr);
}

uint getOptimalPosition(VOC * voc, uint word) {
	return vector_get(voc->o2s, word);
}

hash_entry insertWord(VOC * voc, unsigned char * word, unsigned int * addr) {

	uint pos = 0, sortedPos = voc->nWordsReal;
	hash_entry he;
	uint found = 0;
	while(!isEmptyStack(voc->emptyCodes)) {
		pos = popStack(&(voc->emptyCodes));
//		DEBUG("Reusing word %d\n", pos);
		if (!vector_get(voc->freqs, pos)) {found = 1;break;}
		else {
			printf("empty code with positive frequency \n");
			exit(EXIT_UNEXPECTED);
		}
	}
	if (!found) {
		pos = voc->nWords++;
	}
	voc->nWordsReal++;

	he = insertElement(voc->inverse, word, addr, pos);

	vector_set(voc->posInTH, pos, *addr);

	vector_set(voc->o2s, pos, sortedPos);
	vector_set(voc->s2o, sortedPos, pos);
	vector_set(voc->freqs, pos, 0);


	return he;

}

uint getOptimalSize(VOC * voc) {
	if (voc->readonly) {
		DEBUG("Voc loaded as read-only. Optimal size is not known!\n");
		return 0;
	}
	int i;
	uint size = 0;
	for (i = 0; i < voc->nWordsReal; i++) {
		size += byteSize(i) * vector_get(voc->freqs, vector_get(voc->s2o, i));
	}
	return size;
}

uint getCompressedSize(VOC * voc) {
	if (voc->readonly) {
		DEBUG("Voc loaded as read-only. Freqs are not loaded!\n");
		return 0;
	}
	int i;
	uint size = 0;
	for (i = 0; i < voc->nWords; i++) {
		size += byteSize(i) * vector_get(voc->freqs,i);
	}

	return size;
}

uint getFixedSize(VOC * voc) {
	int i, j;
	uint size = 0;
	for (i = 0; i < voc->nWords; i++) {
		byte * w = getWordVocabulary(voc, i);
		int nones = 0;
		for (j = 0; j < voc->wordLength; j++) {
			nones += popcount(w[j]);
		}
		if (nones == 1) size += vector_get(voc->freqs, i);
		else size += (1 + voc->wordLength) * vector_get(voc->freqs, i);
	}
	return size;
}

void swapVoc(VOC * voc, uint curPos, uint newSPos) {
	uint curSPos = vector_get(voc->o2s, curPos);
	uint newPos = vector_get(voc->s2o, newSPos);
	voc->optimalSize +=  (long)(byteSize(curSPos)-byteSize(newSPos))*vector_get(voc->freqs, newPos) + (long)(byteSize(newSPos)-byteSize(curSPos))*vector_get(voc->freqs, curPos);
	uint aux = vector_get(voc->o2s, curPos);
	vector_set(voc->o2s, curPos, vector_get(voc->o2s, newPos));
	vector_set(voc->o2s, newPos, aux);
	aux = vector_get(voc->s2o, newSPos);
	vector_set(voc->s2o, newSPos, vector_get(voc->s2o, curSPos));
	vector_set(voc->s2o, curSPos, aux);

}

void increaseFrequency(VOC * voc, uint pos) {
	uint f;
	f = vector_get(voc->freqs, pos);

	vector_set(voc->freqs, pos, vector_get(voc->freqs, pos) + 1);
	voc->compressedSize += byteSize(pos);
	voc->optimalSize += (long)byteSize(vector_get(voc->o2s, pos));

	int curTop = vector_get(voc->top, f);
	swapVoc(voc, pos, curTop);
	vector_set(voc->top, f, curTop+1);
	if (voc->maxFreq == f+1) {
		vector_set(voc->top, f+1, 0);
		voc->maxFreq++;
	}

}

ull addEntryVocabulary(VOC * voc,  unsigned char * word) {
	if (voc->inverse->numElem >= voc->inverse->maxElem - 1) {
		hash_table old = voc->inverse;
//		DEBUG("ne %d, nw %d\n", old->numElem, voc->nWords);
		hash_table newtable = initialize_hash(old->maxElem * 2, old->keySize);
		DEBUG("Rebuilding hash table! New size : %d (had %d elements)\n", old->maxElem * 2, old->numElem);
		voc->inverse = newtable;
		uint i;
		for (i = 0; i < voc->nWords; i++) {
			if (vector_get(voc->freqs, i)) {
				uint myaddr;
				search(voc->inverse, svector_get(old->words, i), &myaddr);
				insertElement(voc->inverse, svector_get(old->words, i), &myaddr, i);
				vector_set(voc->posInTH, i, myaddr);
//				if (!i) DEBUG("myaddr = %d, value = %d\n", myaddr, i);

//				insertWord(voc, svector_get(old->words, i), &myaddr);
			}
		}

//		for (i = 0; i < voc->nWords; i++) {
//			if (vector_get(voc->freqs, i)) {
//				uint myaddr;
//				hash_entry newhe = search(voc->inverse, svector_get(old->words, i), &myaddr);
//				if (!i) DEBUG("myaddr = %d, value = %d\n", myaddr, newhe->value);
//				hash_entry oldhe = search(old, svector_get(old->words, i), &myaddr);
//				if (!i) DEBUG("myaddr = %d, value = %d\n", myaddr, oldhe->value);
//				if (newhe->value != oldhe->value)
//					exit(1/0);
////				insertElement(voc->inverse, svector_get(old->words, i), &myaddr, i);
////				insertWord(voc, svector_get(old->words, i), &myaddr);
//			}
//		}


		freeHashTable(old, 1);
//		DEBUG("Rebuilt! Has %d elements\n", voc->inverse->numElem);
	}

	uint addr;
	hash_entry he = search(voc->inverse, word, &addr);
	if (!ISFOUND(he)) {
		insertWord(voc, word, &addr);
	}
	increaseFrequency(voc, he->value);
	return he->value;
}



void removeEntryVocabulary(VOC * voc, unsigned char * word) {
	uint addr;
	hash_entry he = search(voc->inverse, word, &addr);
	uint pos = he->value;

//	if (!vector_get(voc->freqs, pos)) exit(1/0);
	uint f = vector_get(voc->freqs, pos);

	vector_set(voc->freqs, pos, f-1);
	voc->compressedSize -= byteSize(pos);
	voc->optimalSize -= byteSize(vector_get(voc->o2s, pos));


	if (vector_get(voc->freqs, pos) == 0) {
		deleteElement(voc->inverse, addr);
		pushStack(&(voc->emptyCodes), pos);
		voc->nWordsReal--;
	}

	uint newPos = f>1?vector_get(voc->top,f-1)-1:voc->nWordsReal;
	swapVoc(voc, pos, newPos);
	if (vector_get(voc->top,f-1)) vector_set(voc->top,f-1, vector_get(voc->top, f-1) - 1);
}

void replaceByOptimal(VOC * voc) {
	int i;

	for (i = 0; i < voc->nWords; i++) {
		voc->inverse->table[vector_get(voc->posInTH, i)].value = vector_get(voc->o2s, i);
	}

//	uint newsize = 1 << (bits(voc->maxSize));
	svector * nwords = svector_init2(voc->inverse->words->maxEls, voc->inverse->keySize + 1);
	for (i = 0; i < voc->nWords; i++) {
		svector_set(nwords, i, svector_get(voc->inverse->words, vector_get(voc->s2o, i)));
	}

	//We use s2o to store the new positionInTH
	//We use o2s to store the new freqs
	for (i = 0; i < voc->nWords; i++) {
		vector_set(voc->o2s, i, vector_get(voc->freqs, vector_get(voc->s2o, i)));
		vector_set(voc->s2o, i, vector_get(voc->posInTH, vector_get(voc->s2o, i)));
	}

//	Replace array words!!!!
	svector_destroy(voc->inverse->words);
	voc->inverse->words = nwords;



	voc->nWords = voc->nWordsReal;
	for (i = 0; i < voc->nWords; i++) {
		vector_set(voc->freqs, i, vector_get(voc->o2s, i));
		vector_set(voc->posInTH, i, vector_get(voc->s2o, i));
//		voc->inverse->table[vector_get(voc->posInTH, i)].value = i;
//		he->value = i;
		vector_set(voc->o2s, i, i);
		vector_set(voc->s2o, i, i);
	}

//	for (i = 0; i < voc->nWords-1; i++) {
//		if (voc->freqs[i] < voc->freqs[i+1]) exit(EXIT_UNEXPECTED);
//	}

	voc->compressedSize = voc->optimalSize;
	emptyStack(&voc->emptyCodes);
}


