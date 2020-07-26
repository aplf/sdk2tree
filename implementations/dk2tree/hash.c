/* Searches using  Horspool's algorithm adapted to 
 search inside a text compressed with End-Tagged Dense Code.
 Lightweight Natural Language Text Compression: Information Retrieval 2006

 Programmed by Antonio Faria.

 Author's contact: Antonio Faria, Databases Lab, University of
 A Corua. Campus de Elvia s/n. Spain  fari@udc.es

 Copyright (C) 2006  Nieves R. Brisaboa, Gonzalo Navarro, Antonio Faria and Jos R. Param
 Author's contact: antonio.fari@gmail.com

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 aint with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*-----------------------------------------------------------------------
 Hash.c: Definition of a HashTable that uses linear hash
 ------------------------------------------------------------------------*/

#include "hash.h"

#define mystrncpy(a,b,n) {unsigned int i; for(i=0;i<n;i++) a[i]=b[i];}

unsigned int NearestPrime(unsigned int n);

//uint mmcurblock = 0;



/*-----------------------------------------------------------------
 Initilization of data structures used by the hashTable
 ---------------------------------------------------------------- */
hash_table initialize_hash(unsigned int sizeVoc, uint keySize) {

	hash_table hash;
	unsigned int i;

	hash = (hash_table) malloc(sizeof(struct Ht));
	hash->keySize = keySize;
	hash->maxElem = sizeVoc;
	hash->numElem = 0;
	hash->hashSize = NearestPrime(OCUP_HASH * hash->maxElem);
	hash->table = (struct entry *) malloc(hash->hashSize * sizeof(struct entry));
	hash->words = svector_init2(sizeVoc, hash->keySize + 1);

//	hash->memMgr = mgr;

	for (i = 0; i < hash->hashSize; i++) {
//		hash->table[i].key = NULL;
		hash->table[i].status = STATUS_EMPTY;
	}
//	mmcurblock = mgr->currentBlock;
	return hash;
}

/*-----------------------------------------------------------------
 Frees the memory allocated by the hash table 
 -----------------------------------------------------------------*/
void freeHashTable(hash_table hash, uint freeMemMgr) {
//	if (freeMemMgr) {
//		destroyMemoryManager(hash->memMgr);
//		free(hash->memMgr);
//	}
	free(hash->table);
	svector_destroy(hash->words);
	free(hash);
}

/*------------------------------------------------------------------
 Find the nearest prime number over n. 
 ---------------------------------------------------------------- */
unsigned int NearestPrime(unsigned int n) {
	int position; /* the prime number being sought */
	int index;

	for (position = n;; position++) {
		/* checks if those values from 2 to $\sqrt{m}$ can be factors of $m$ */
		for (index = 2; index <= (int) sqrt((double) position) && position
				% index != 0; index++)
			;

		if (position % index != 0) /* No factors in that range, therefore a prime number was found */
		{
			break;
		}
	}
	return position;
}

/*------------------------------------------------------------------
 Modification of Zobel's bitwise function to have into account the 
 lenght of the key explicetely 
 ---------------------------------------------------------------- */
unsigned int hashFunction(hash_table hash, const unsigned char *aWord) {
	unsigned char c;
	unsigned int h;
	unsigned long last;
	last = ((unsigned long) aWord) + hash->keySize - 1;

	h = SEED;

	for (; ((unsigned long) aWord <= last);) {
		c = *(aWord++);
		/*c=*aWord;*/
		h ^= ((h << 5) + c + (h >> 2));
	}
	return ((unsigned int) ((h & 0x7fffffff) % hash->hashSize));
}

/*------------------------------------------------------------------
 Modification of Zobel's scmp function compare two strings
 ---------------------------------------------------------------- */
/*inline*/ int strcomp(const unsigned char *s1, const unsigned char *s2,
		register unsigned int ws1, unsigned int ws2) {

	if (ws1 != ws2) {
		return -1;
	}

	{
		register unsigned int iters;
		iters = 1;
		while (iters < ws1 && *s1 == *s2) {
			s1++;
			s2++;
			iters++;
		}

		return (*s1 - *s2);
	}
}

/*-----------------------------------------------------------------------
 Inserts a new word in a given position of the hashTable (position previously computed)
 ---------------------------------------------------------------------*/

hash_entry insertElement(hash_table hash, unsigned char *aWord,
		uint * addr, uint value) {
//	uint len = hash->keySize;

//	getMemoryBlock(hash->memMgr, (byte **) &(hash->table[*addr].key), len + 1);

	{
		svector_set(hash->words, value, aWord);
	}

	hash->table[*addr].value = value;
	hash->table[*addr].status = STATUS_OK;
	hash->numElem++;

	return &(hash->table[*addr]);
}

void deleteElement(hash_table hash, uint origAddr) {
	hash->table[origAddr].status = STATUS_DELETED;
	hash->numElem--;
}

/*-----------------------------------------------------------------------
 Searches for a word in the hash table and returns its position in the
 vocabulary. It returns the next "available" position in the vocabulary,
 if the word is not in the hash table. That is: a "0-node" position.
 It also returns -using attribute returnedAddr- the position where word
 was found (or where it should go if it was inserted in next "insert call".
 -----------------------------------------------------------------------*/
hash_entry search(hash_table hash, const unsigned char *aWord, unsigned int *returnedAddr) {

	register unsigned int addr;
	register int insertAddr = -1;

	addr = hashFunction(hash, aWord);

	while (((hash->table[addr].status != STATUS_EMPTY) && ((strcomp(svector_get(hash->words, hash->table[addr].value), aWord, hash->keySize, hash->keySize)) != 0))
			|| hash->table[addr].status == STATUS_DELETED) {
		if (hash->table[addr].status == STATUS_DELETED) {
			insertAddr = addr;
			printf(".");
		}
		addr = ((addr + JUMP) % hash->hashSize);
		printf("%x ", addr);
	}
	if (hash->table[addr].status == STATUS_EMPTY && insertAddr != -1)
		addr = insertAddr;

	*returnedAddr = addr; /* position returned*/

	return &(hash->table[addr]);

}


uint memUsageHt(hash_table hash, uint includeMM) {
	uint size = 0;
	size += sizeof(struct Ht);
	size += hash->hashSize * sizeof(struct entry);
	size += svector_size(hash->words)*hash->words->elSize;
//	size += getMemoryUsage(hash->memMgr);
	printf("Memory usage of hash: %d \n", size);
	return size;
}
