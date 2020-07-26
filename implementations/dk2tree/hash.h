/*
 *
 * Adapted from Antonio Fariña's (below)
 *
 */

/*-----------------------------------------------------------------------
 Hash.h: Definition of a HashTable that uses linear hash
 ------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <malloc.h>

#include "util.h"

#include "MemoryManager.h"

#define JUMP 101  		 	/*jump done when a collision appears*/
#define OCUP_HASH 1.5	 	/*index of occupation of the hash table*/
#define SMALL_PRIME 1009 	/*a small prime number, used to compute a hash function*/
#define SEED	1159241
/* Type definitions */

#define BASE_HASH_SIZE 8192

#define STATUS_OK 0
#define STATUS_DELETED 1
#define STATUS_EMPTY 2

#define MIN(a,b) (a < b) ? a : b
#ifndef uint
	typedef unsigned int uint;
#endif
#define ISFOUND(h) (he->status == STATUS_OK)

struct entry {
//	unsigned char * key;
	uint value;
	unsigned char status;
};
typedef struct entry *hash_entry;

struct Ht {
	MemoryManager memMgr;
	uint maxElem;
	uint numElem;
	int keySize;
	uint hashSize;
	struct entry *table;
	svector * words;
};
typedef struct Ht *hash_table;

unsigned int hashFunction(hash_table hash, const unsigned char *aWord);

hash_table initialize_hash(unsigned int tamArq, uint keySize);
hash_table resize_hash(hash_table old, uint newSize);
void freeHashTable(hash_table hash, uint freeMemMgr);
hash_entry insertElement(hash_table hash, unsigned char *aWord,
		register unsigned int *addr, uint value);
void deleteElement(hash_table hash, unsigned int addr);
hash_entry search(hash_table hash, const unsigned char *aWord,
		unsigned int *returnedAddr);
uint memUsageHt(hash_table hash, uint includeMM);

uint saveHash(hash_table hash, char * fileName);
hash_table loadHash(char * fileName);


