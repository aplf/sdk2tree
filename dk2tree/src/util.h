
#ifndef ___UTIL_H___
#define ___UTIL_H___

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "basic.h"
#include "debug.h"


typedef unsigned long ull;
//typedef unsigned long long ull;

#define DEF_BIG_SIZE 50000000
#define DEF_SMALL_SIZE 256
#define DEF_VECTOR_SIZE 8192
//#define DEF_VECTOR_SIZE 65536

#define BYTES(bits) ((bits)/8 + ((bits%8)>0))

#define MEMORY_EXCEEDED (6)
#define IO_ERROR (-2)
#define EXIT_UNEXPECTED (-1)

void * MALLOC(uint size);

#define FREAD(p,s,e,f) {int res = fread(p,s,e,f); if (res != (e)) {ERROR("Unable to read file\n");exit(IO_ERROR);}}

#define N_SB 16384
#define SB_SIZE 256

typedef struct sIntStack {
	int *values;
	int top;
	int maxSize;
} IntStack;


void initStack(IntStack *stack, int maxSize);

void emptyStack(IntStack *stack);

void pushStack(IntStack *stack, int value);

int popStack(IntStack *stack);

int isEmptyStack(IntStack stack);

void freeStack(IntStack stack);

int isInStack(IntStack stack, int value);

int stackSize(IntStack stack);


typedef struct sVector {
	uint * values;
	uint maxSize;
} vector;

//typedef struct sVector {
//	uint ** values;
//	uint maxSize;
//} vector;

//typedef struct sVector {
//	uint ** vector;
//	uint nsub;
////	uint size;
//} vector;

vector * vector_init(void);
vector * vector_init2(uint size);
void vector_destroy(vector *v);
uint vector_size(vector *v);
int vector_isEmpty(vector * v);
uint vector_get(vector *v, uint pos);
void vector_set(vector *v, uint pos, uint val);
void vector_sort(vector *v, uint size, __compar_fn_t compare);


typedef struct s_vector {
	unsigned char * values;
	uint maxEls;
	uint elSize;
} svector;

svector *svector_init(uint elSize);
svector *svector_init2(uint maxEls, uint elSize);
void svector_destroy(svector *v);
unsigned char * svector_get(svector *v, uint pos);
void svector_set(svector *v, uint pos, unsigned char * val);
uint svector_size(svector * sv);
//inline void svector_sort(svector *v, uint size, __compar_fn_t compare);



FILE * createFile(char * fileName);

/*
 * Open file in read-write mode, create if does not exist
 */
FILE * openFile(char * fileName);

/*
 * Make a space of size bits, starting at bit pos, into data.
 * dataSize keeps the size of data in bits.
 */
void makeSpace(uint * data, uint dataSize, uint pos, uint size);

void deleteSpace(uint * data, uint dataSize, uint pos, uint size);

uint _countOnes(uint *A, register int x);
uint _countOnes2(uint *A, register int start, register int end);

void bit_memcpy(uint *dest, uint bitdest, uint * source, uint bitsource, uint bits, int ignoreAfter);
//void bit_memcpy1(uint *dest, uint bitdest, uint * source, uint bitsource, uint bits);
//void bit_memcpy2(uint *dest, uint bitdest, uint * source, uint bitsource, uint bits);

#endif
