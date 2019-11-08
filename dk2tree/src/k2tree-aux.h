#ifndef K2TREE_AUX
#define K2TREE_AUX

#define DEF_QUEUE_MAX 50000000

typedef struct sSimple {
	ull position;
	ull row;
	ull column;
} SINGLE_QNODE;

typedef struct sQueueSimple {
	SINGLE_QNODE els[DEF_QUEUE_MAX];
	int ini;
	int fin;
} SINGLE_QUEUE;

SINGLE_QUEUE * getsq() {
	SINGLE_QUEUE * ret = (SINGLE_QUEUE *) malloc(sizeof(SINGLE_QUEUE));
	ret->ini=DEF_QUEUE_MAX -1;
	ret->fin = 0;
	return ret;
}

int emptysq(SINGLE_QUEUE * queue) {
	return fin == (ini + 1) % DEF_QUEUE_MAX;
}

void pushsq(SINGLE_QUEUE * queue, ull position, ull row, ull column) {
	queue->ini++;
	queue->els[ini].position = position;
	queue->els[ini].row = row;
	queue->els[ini].column = column;
}
SINGLE_QNODE popsq(SINGLE_QUEUE * queue) {
	queue->fin = (queue->fin + 1) % DEF_QUEUE_MAX;
}
SINGLE_QNODE popsqn(SINGLE_QUEUE * queue, int n);


#define addQueue(q, v)

#endif
