#include "k2tree-common.h"
#include "k2tree-impl.h"
#include "encoding.h"
#include "static/kTree.h"

uint *globalbitmap;
uint *globalbitmap2;
long globalpos = 0;
uint globalnleaves = 0;
uint * globalli;
long curli = 0L;
void initbitmap(K2Tree * k2tree) {
//	globalbitmap = (uint *) malloc(100000000);
	globalpos = 0L;
	globalbitmap = (uint *) malloc(256000000*sizeof(uint));
	globalbitmap2 = (uint *) malloc(256000000*sizeof(uint));
}
void foo(K2Tree *k2tree) {}
void foo2(K2Tree * k2tree, Node * node) {
//	printf("entering node %d\n", node->bid);
}

int vistas = 0;
void acumLeaf(K2Tree * k2tree, Node * node) {
//	if (vistas ++ < 1) {
//		printf("processing leaf %d. size  = %d\n", node->bid, GETSIZE(node->data[0]));
		if (k2tree->inFirstLevels) {
			uint size = GETSIZE(node->data[0]);
			int samplesBits = getNSamples(k2tree, NODETYPE(node->bid)) * SAMPLESIZEBITS1;
//			printNode(k2tree, node);
			bit_memcpy(globalbitmap, globalpos, ((uint *)(node->data))+1, samplesBits, size, 1);
			globalpos += size;
		} else {
			uint size = GETSIZE(((uint *)(node->data))[0]);
			unsigned char * data = (unsigned char *)(((uint *)(node->data))+1);
			uint totalread = 0;
			uint offset = getNSamples(k2tree, NODETYPE(node->bid)) * SAMPLESIZE2;
			while(totalread * 8 < size) {
//				printf(".");
				uint bytesRead = 0;
				uint val = decodeValue(data+offset+totalread, &bytesRead);
				globalli[curli++] = val;
				totalread += bytesRead;
			}
		}
//	}
}

void getInfo(K2Tree * k2tree) {

	k2tree->inFirstLevels = 1;
	doSomething(k2tree, initbitmap, foo2, acumLeaf, foo);
	printf("got %ld bits\n", globalpos);

	int sizebt = k2tree->startLevels[k2tree->nLevels - 2];
	//Copiamos valores para BN (último nivel)
	bit_memcpy(globalbitmap2, 0, globalbitmap, sizebt,
			k2tree->startLevels[k2tree->nLevels -1] - k2tree->startLevels[k2tree->nLevels - 2], 1);

//	printf("%d bits copied to second bitmap.",k2tree->startLevels[k2tree->nLevels -1] - k2tree->startLevels[k2tree->nLevels - 2]);

	int i;
	for (i = sizebt; i < sizebt + 32; i++) {
		bitclean(globalbitmap, i);
	}

	printf("distributing %d bits to bn\n", k2tree->startLevels[k2tree->nLevels -1] - k2tree->startLevels[k2tree->nLevels - 2]);

	k2tree->inFirstLevels = 0;

	Node * node = readNode(k2tree->mgr, k2tree->bidroot2);
//	printf("root2 = %d\n", node->bid);
	uint size = GETSIZE(((uint *)node->data)[0]);
	uint nentries = size / ENTRYSIZEBITS2;
	printf("size = %d, entries = %d\n", size, nentries);
//	printNode(k2tree, node);
//	int i;
	int ltreesize = 0;
	for (i = 0; i < nentries; i++)
		globalnleaves += GETNENTRIES(0, node,i) / ENTRYSIZEBITS2;
	printf("size of LTree : %d leaves\n", globalnleaves);
	fflush(stdout);

	globalli = (uint *) malloc((long)globalnleaves * sizeof(uint));

	k2tree->inFirstLevels = 0;
	doSomething(k2tree, foo, foo2, acumLeaf, foo);

	printf("curli = %d\n", curli);

}

TREP * buildStatic(K2Tree * k2tree) {
	int tamSubm = k2tree->nNodes;
	int i, j;
	uint nodes = k2tree->nNodesReal;
	uint part = 1;

	int max_level1;
	int k1 = k2tree->kValues[0];
	int max_real_level1 = 0; //Valor innecesario no k2-tree estático. Poño un valor por defecto para facer tests
	int itt = 0;

	while(k2tree->kValues[itt++] == k2tree->kValues[0]);
	int k2 = k2tree->kValues[itt];
	max_level1 = itt-1;

	uint curk = k2tree->kValues[k2tree->nLevels-1];
	uint prevk = k2tree->kValues[k2tree->nLevels-2];
	uint lval = 0;
	while(curk > prevk) {
		lval++;
		curk /= prevk;
	}

	int max_level2 = k2tree->nLevels - max_level1 + (lval -1);

	for (i = 0; i < k2tree->nLevels; i++) {
		printf("dl[%d] = %d\n", i, k2tree->divLevels[i]);
	}

	for (i = 0; i < k2tree->nLevels; i++) {
		printf("ks[%d] = %d\n", i, k2tree->kValues[i]);
	}



	printf("lval = %d\n", lval);

	printf("got %d %d levels", max_level1, max_level2);

	TREP * trep = createTreeRep(k2tree->nNodesReal, k2tree->nEdges, part, tamSubm, max_real_level1, max_level1, max_level2, k1, k2, lval);

	trep->submatrices[0][0] = (MREP *) malloc(sizeof(struct matrixRep));

	trep->submatrices[0][0]->numberOfNodes = tamSubm;
	trep->submatrices[0][0]->numberOfEdges = trep->numberOfEdges;
	trep->submatrices[0][0]->cutBt = k2tree->startLevels[max_level1];
	trep->submatrices[0][0]->lastBt1_len = k2tree->startLevels[max_level1-1];

	getInfo(k2tree);

	trep->submatrices[0][0]->bt_len = k2tree->startLevels[k2tree->nLevels - 2];
	trep->submatrices[0][0]->bn_len = k2tree->startLevels[k2tree->nLevels - 1] - k2tree->startLevels[k2tree->nLevels - 2];
	trep->submatrices[0][0]->nleaves = globalnleaves;
	trep->submatrices[0][0]->cutBt = k2tree->startLevels[trep->maxLevel1];
	trep->submatrices[0][0]->lastBt1_len = k2tree->startLevels[trep->maxLevel1-1];

	trep->submatrices[0][0]->bt = createBitRankW32Int(globalbitmap, trep->submatrices[0][0]->bt_len, 1, 4);

	trep->submatrices[0][0]->bn = createBitRankW32Int(globalbitmap2, trep->submatrices[0][0]->bn_len, 1, 20);
	trep->submatrices[0][0]->leavesInf = NULL;

//	free(globalbitmap);
//	free(globalbitmap2);

//	trep->submatrices[0][0]->compressIL = createFT(NULL, -666);

	trep->submatrices[0][0]->compressIL = createFT(globalli, globalnleaves);

	trep->zeroNode = k2tree->voc->nWords;
	trep->lenWords = k2tree->voc->wordLength;
	trep->words = (unsigned char *) malloc(trep->zeroNode * trep->lenWords);
	for (i = 0; i < trep->zeroNode; i++) {
		char * word = getWordVocabulary(k2tree->voc, i);
		memcpy(trep->words + i*trep->lenWords, word, trep->lenWords);
	}

	printf("voc has %d words\n");

//	{
//		int itt;
//		FILE * out = fopen("iltrans", "w+");
//		for (itt = 0; itt < trep->submatrices[0][0]->nleaves; itt++) {
//			uint pos = accessFT(trep->submatrices[0][0]->compressIL, itt+1);
//			char * word = getWordVocabulary(k2tree->voc, pos);
//			fwrite(word, sizeof(char), trep->lenWords, out);
//			if (itt == 0)
//				printWordVocabulary(k2tree->voc, pos);
//		}
//		fclose(out);
//	}

	return trep;
}



int main(int argc, char **argv) {
	if (argc < 3) {
		printf("usage: %s <input> <output>\n", argv[0]);
		exit(EXIT_UNEXPECTED);
	}
	K2Tree * tree = loadK2Tree(argv[1], 0);


	TREP * out = buildStatic(tree);

	saveTreeRep(out, argv[2]);
	destroyTreeRepresentation(out);
	destroyK2Tree(tree);
	exit(0);
}
