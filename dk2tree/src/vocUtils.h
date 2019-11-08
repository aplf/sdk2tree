#ifndef __VOCUTILS_H__

#define __VOCUTILS_H__

#include "basic.h"
#include "hash.h"
#include "util.h"

#define MAX_WORDS_VOC 5000000
#define MAX_FREQS_VOC 5000000

typedef struct svoc {
	uint maxSize;
	uint nWords;
	uint nWordsReal;
	uint wordLength;
	vector * posInTH;
	hash_table inverse;
	vector * freqs;
	IntStack emptyCodes;
	int readonly;
	uint compressedSize;
	uint optimalSize;
	vector *o2s;
	vector *s2o;
	vector *top;
	uint maxFreq;
	unsigned char * words;
} VOC;

VOC * loadVocabulary(char * basename, uint readonly);
VOC * createEmptyVocabulary(uint wordLen);

void freeVocabulary(VOC * voc);

void saveVocabulary(VOC * voc, char * filename);

uint memUsageVocabulary(VOC * voc);
uint diskUsageVocabulary(VOC * voc);

unsigned char * getWordVocabulary(VOC * voc, uint pos);

uint getOptimalPosition(VOC * voc, uint word);

void printWordVocabulary(VOC * voc, uint pos);

hash_entry getEntryWord(VOC * voc, unsigned char * word, unsigned int * addr);

hash_entry insertWord(VOC * voc, unsigned char * word, unsigned int * addr);

ull addEntryVocabulary(VOC * voc, unsigned char * word);
void removeEntryVocabulary(VOC * voc, unsigned char * word);

uint getCompressedSize(VOC *voc);
uint getOptimalSize(VOC *voc);

void replaceByOptimal(VOC * voc);

#endif
