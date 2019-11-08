#include "util.h"

/*
void emptyStack(IntStack * stack, int maxSize) {
	*stack = NULL;
}

void pushStack(IntStack *stack, int value) {
	IntNode node = (IntNode)malloc(sizeof(IntN));
	node->value = value;
	node->next = *stack;
	*stack = node;
}

int popStack(IntStack *stack) {
	IntNode n = *stack;
	int ret = n->value;
	*stack = n->next;
	return ret;
}

int isEmptyStack(IntStack stack){
	return stack==NULL;
}

void freeStack(IntStack stack) {
	return;
}
*/

void initStack(IntStack *stack, int maxSize) {
	stack->maxSize = maxSize;
	stack->values = (int *)calloc(maxSize, sizeof(int));
	emptyStack(stack);
}

void emptyStack(IntStack * stack) {
	stack->top = -1;
}

void pushStack(IntStack *stack, int value) {
	if (stack->top >= stack->maxSize - 1) {
		stack->maxSize *= 2;
//		printf("Resizing stack to size %d\n", stack->maxSize);
		stack->values = realloc(stack->values, stack->maxSize * sizeof(uint));
	}
	stack->values[++stack->top] = value;
}

int popStack(IntStack *stack) {
//	if (stack->top <= stack->maxSize / 3) {
//		stack->maxSize /= 2;
//		stack->values = realloc(stack->values, stack->maxSize);
//	}
	return stack->values[stack->top--];

}

int isEmptyStack(IntStack stack){
	return stack.top < 0;
}

void freeStack(IntStack stack) {
	free(stack.values);
}

int isInStack(IntStack stack, int value) {
	int i;
	for (i = 0; i <= stack.top; i++) {
		if (stack.values[i] == value) return 1;
	}
	return 0;
}

int stackSize(IntStack stack) {
	return stack.top <0 ? 0 : stack.top + 1;
}

void printStack(IntStack stack) {
	int i;
	printf("[");
	for (i = 0; i <= stack.top; i++) {
		printf("%04d ", stack.values[i]);
	}
	printf(">\n");
}

vector * vector_init2(uint size){
	vector *v = (vector *) MALLOC(sizeof(vector));
	v->maxSize = size;
	v->values = (uint *) calloc(v->maxSize, sizeof(uint));
	return v;
}
vector *vector_init() {
	return vector_init2(DEF_VECTOR_SIZE);
}
void vector_destroy(vector *v) {
	free(v->values);
	free(v);
}
uint vector_size(vector *v) {
	return v->maxSize;
}
int vector_isEmpty(vector * v) {
	return !v->maxSize;
}
uint vector_get(vector *v, uint pos) {
	return v->values[pos];
}
void vector_set(vector *v, uint pos, uint val) {
	if (pos >= v->maxSize) {
		v->maxSize *= 2;
//		DEBUG("Resizing vector. New size : %d\n", v->maxSize);
		v->values = (uint *) realloc(v->values, v->maxSize * sizeof(uint));
	}
	v->values[pos] = val;
}

void vector_sort(vector *v, uint size, __compar_fn_t compare) {
	qsort(v->values, size, sizeof(uint), compare);
}


//inline vector * vector_init(uint size){
//	vector *v = (vector *) MALLOC(sizeof(vector));
//	v->values = (uint **) calloc(N_SB, sizeof(uint *));
//	v->values[0] = (uint *) MALLOC(SB_SIZE * sizeof(uint));
//	v->maxSize = size;
//	return v;
//}
//inline void vector_destroy(vector *v) {
//	uint i;
//	for (i = 0; i < N_SB && v->values[i]; i++) {
//		free(v->values[i]);
//	}
//	free(v->values);
//	free(v);
//}
//inline uint vector_size(vector *v) {
//	return v->maxSize;
//}
//inline int vector_isEmpty(vector * v) {
//	return !v->maxSize;
//}
//inline uint vector_get(vector *v, uint pos) {
//	return v->values[pos/SB_SIZE][pos%SB_SIZE];
//}
//inline void vector_set(vector *v, uint pos, uint val) {
//	int nsb = pos/SB_SIZE;
//	if (!v->values[nsb])
//		v->values[nsb] = (uint *) MALLOC(SB_SIZE * sizeof(uint));
//	v->values[nsb][pos%SB_SIZE] = val;
//}
//
//inline void vector_sort(vector *v, uint size, __compar_fn_t compare) {
//	//FIXME:
//	qsort(v->values, size, sizeof(uint), compare);
//}


svector *svector_init2(uint maxEls, uint elSize) {
	svector *sv = (svector *) MALLOC(sizeof(svector));
	sv->maxEls = maxEls;
	sv->elSize = elSize;
	sv->values = (unsigned char *) MALLOC(sv->maxEls * sv->elSize * sizeof(unsigned char));
	return sv;
}

svector *svector_init(uint elSize) {
	return svector_init2(DEF_VECTOR_SIZE, elSize);
}

void svector_destroy(svector *sv) {
	free(sv->values);
	free(sv);
}
uint svector_size(svector * sv) {
	return sv->maxEls;
}


unsigned char * svector_get(svector *v, uint pos) {
	return v->values + pos * v->elSize;
}
void svector_set(svector *v, uint pos, unsigned char * val) {
	if (pos > v->maxEls) {
		v->maxEls *=2;
		v->values = realloc(v->values, v->maxEls * v->elSize * sizeof(unsigned char));
	}
	memcpy(v->values+pos*v->elSize, val, v->elSize - 1);
	v->values[pos*v->elSize + v->elSize - 1] = '\0';
}

//inline svector *svector_init(uint maxEls, uint elSize) {
//	svector *sv = (svector *) MALLOC(sizeof(svector));
//	sv->values = (unsigned char *) MALLOC(maxEls * elSize * sizeof(unsigned char));
//	sv->elSize = elSize;
//	return sv;
//}
//inline void svector_destroy(svector *sv) {
//	free(sv->values);
//	free(sv);
//}
//
//
//inline unsigned char * svector_get(svector *v, uint pos) {
//	return v->values + pos * v->elSize;
//}
//inline void svector_set(svector *v, uint pos, unsigned char * val) {
//	memcpy(v->values+pos*v->elSize, val, v->elSize - 1);
//	v->values[pos*v->elSize + v->elSize - 1] = '\0';
//}

FILE * createFile(char * fileName) {
	FILE *f = fopen (fileName, "w+");
	if (!f) {
		ERROR("Could not create file %s\n", fileName);
		exit(IO_ERROR);
	}
	return f;
}

FILE * openFile(char * fileName) {
	FILE *f = fopen (fileName, "r+");
//	DEBUG("Opening file %s\n", fileName);
	if (!f) {
		DEBUG("Cannot open file. Trying to create a new one\n");
		f = createFile(fileName);
	}
	return f;
}

void bit_memcpy(uint *dest, uint destOffset, uint *source, uint sourceOffset, uint len, int ignoreAfter) {

	if (len <= 32) {
		int i;
		for (i = 0; i < len; i++) {
			if(bitget(source, sourceOffset+i)) bitset(dest, destOffset+i);
			else bitclean(dest, destOffset+i);
		}
	} else {
		if ( !(destOffset%8) && !(sourceOffset%8) && ( !(len%8) || ignoreAfter) ) {
			memcpy(((byte *)dest)+destOffset/8, ((byte *)source)+sourceOffset/8, (len+7)/8);
		} else {
			uint wordDest = destOffset / W;
			uint modDest = destOffset % W;
			uint wordSource = sourceOffset / W;
			uint modSource = sourceOffset % W;

			source += wordSource;
			dest += wordDest;

			uint x;

			if (modSource == modDest) {

				if (modSource) {
					x = *source++ & (~0u << modSource);

					if (len + modSource >= 32) {
						*dest &= modSource ? ~0u >> (32 - modSource) : 0;
						len -= (32 - modSource);
					} else {
						*dest &= (modSource ? ~0u >> (32 - modSource) : 0) | (~0u << (len + modSource));
						x &= (~0u >> (32 - (len + modSource)));
						len = 0;
					}
					*dest++ |= x;
				}

				int wlen = len / W;
				uint mlen = len % W;

				while(--wlen >= 0) {
					*dest++ = *source++;
				}
				if (mlen) {
					x = *source & (~0u >> (mlen?32-mlen:0));
					*dest = (*dest & (~0u <<mlen)) | (x & (~0u >> (32 - mlen)));
				}

			} else {
				uint bit_diff_ls, bit_diff_rs;
				if (modSource > modDest) {
					bit_diff_ls = modSource - modDest;
					bit_diff_rs = 32 - bit_diff_ls;
					x = *source++ >> bit_diff_ls;
					x |= *source << bit_diff_rs;
					x &= ~0u << modDest;
				} else {
					bit_diff_rs = modDest - modSource;
					bit_diff_ls = 32 - bit_diff_rs;
					x = (*source << bit_diff_rs) & (~0u<<(modDest));
				}

				if (len + modDest >= 32) {
					*dest &= modDest ? ~0u >> (32 - modDest) : 0;
					len -= (32 - modDest);
				} else {
					*dest &= (modDest ? ~0u >> (32 - modDest) : 0) | (~0u << (len + modDest));
					x &= (~0u >> (32 - (len + modDest)));
					len = 0;
				}
				int wlen = len / W;
				uint mlen = len % W;


				*dest++ |= x;

				while(--wlen >= 0) {
					x = *source++ >> bit_diff_ls;
					x |= *source << bit_diff_rs;
					*dest++ = x;
				}
				if (mlen) {
					x = *source++ >> bit_diff_ls;
					if (bit_diff_ls > 32 - mlen) x |= *source << bit_diff_rs;
					x &= ~0u >> (32-mlen);
					*dest = (*dest & (~0u <<mlen)) | x;
				}
			}

		}

	}


}


void makeSpace(uint * data, uint dataSize, uint pos, uint size) {
	int i;

	/*If pos == dataSize we do not need to move anything*/
	if (pos < dataSize) {
		int wordsOffset = size / 32;
		int modSize = size % 32;
		int modPos = pos % 32;
		int lastWord, firstWord;
		lastWord = (dataSize + size - 1) / 32;
	/*	if (dataSize + size > lastWord * 32 ) lastWord++;*/
		firstWord = (pos + size) / 32;

		/*If size is in words make it efficiently*/
		if (!modSize) {
			for (i = lastWord; i >= firstWord; i--) {
				data[i] = data [i-wordsOffset];
			}
		} else {
			for (i = lastWord; i > firstWord; i--) {
				data[i] = data [i-wordsOffset] << modSize | (data [i-wordsOffset - 1] >> (32 - modSize));
			}
			if (((pos + size) % 32) >= (pos%32))
				data[firstWord] = (data[firstWord-wordsOffset] % (1 << modPos)) | ((data[firstWord-wordsOffset] >> modPos) << ((modPos + modSize)%32)) ;
			else {
	/*			printf("firstWord = %d, pos = %d, size =%d\n", firstWord, pos, size);*/
				data[firstWord] = data [firstWord-wordsOffset] << modSize | ((firstWord - wordsOffset > 0) ? (data [firstWord-wordsOffset - 1] >> (32 - modSize)) : 0);
			}

		}
	}

	for (i = pos + size - 1; i >= (int)pos; i--) {
		bitclean(data,i);
	}
}

void deleteSpace(uint * data, uint dataSize, uint pos, uint size) {
//	if (pos + size >= dataSize) return;
	int i;
	int wordsOffset = size / 32;
	int modSize = size % 32;
	int modPos = pos % 32;
	int sumMod = (modPos + modSize)%32;
	int lastWord, firstWord;
	lastWord = (dataSize-1) / 32;
	firstWord = pos / 32;

	if (modSize) {
		if(modPos<=sumMod) {
			data[firstWord] = (data[firstWord] % (1<<modPos)) |
				(((data[firstWord+wordsOffset] >> sumMod) << modPos))| ((firstWord<lastWord)?(data[firstWord+wordsOffset+1]<<(32-modSize)):0);
		} else {
			if (modPos + modSize != 32) {
				data[firstWord] = (data[firstWord] % (1<<modPos)) |  ((data[firstWord+wordsOffset+1]>>sumMod)<<modPos);
			} else {
//				if (firstWord + wordsOffset > 110)
//					DEBUG("%d %d %d\n", dataSize, pos, size);
//				if (firstWord < lastWord)
					data[firstWord] = (data[firstWord] % (1<<modPos)) |  ((data[firstWord+wordsOffset+1])<<modPos);
			}
		}
		for (i = firstWord+1; i < lastWord; i++) {
			data[i] = data [i+wordsOffset] >>  modSize | (data [i+wordsOffset+1] << (32 - modSize));
		}
//		printf("i=%d, fw = %d, lw=%d\n", i, firstWord, lastWord);
		if (i == lastWord)
			data[i]=data[i]>>modSize;
	} else {
		if (modPos)
			data[firstWord] = (data[firstWord+wordsOffset] % (1 << modPos)) | (data[firstWord+wordsOffset] >> modPos) <<modPos;
		else
			data[firstWord] = data[firstWord+wordsOffset];
		for (i = firstWord + 1; i < lastWord; i++) {
			data[i] = data [i+wordsOffset];
		}
	}

}

uint _countOnes(uint *A, register int x) {
	int i;
	int ones = 0;
	for (i = 0; i < x/W; i++){
		ones += popcount(A[i]);
	}
	ones += popcount(A[i]%(1<<x));
//	ones += popcount(A[i]%(1<<(x%W)));
	return ones;
}

uint _countOnes2(uint *A, register int start, register int end) {
	int i;
	int ones = 0;
	int startw = start/W;
	int endw = end/W;
	for (i = startw; i < endw; i++){
		ones += popcount(A[i]);
	}
	ones += popcount(A[i]%(1<<(end%32)));
	if (start % 32) ones -= popcount(A[startw]%(1<<(start%32)));
//	ones += popcount(A[i]%(1<<(end%W)));
//	if (start%W) ones -= popcount(A[startw]%(1<<(start%W)));

	return ones;
}


void * MALLOC(uint size) {
	void * res = malloc(size);
	if (!res) {
		ERROR("Malloc failed!\n");
	}
	return res;
}

