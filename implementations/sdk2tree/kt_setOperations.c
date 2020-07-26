#include <stdio.h>
#include <string.h>
#include "kt_setOperations.h"

/*************************************************************************/
/***************************   FUNCIONES   *******************************/
/***********************  Algoritmos Artículo  ***************************/
/*************************************************************************/


void SkipNodes(uint l, MREP * X, ulong * pX, ulong s);
void fillin(int nlevel, misBits * C);
void preFillin(int nlevel, misBits * C, ulong * limits, uint numNodes, uint maxLevel);
void copy(uint l, MREP * rep, ulong * pX, ulong s, misBits * C);
void limitsCalculator(int cuadrante, ulong * limitsOriginal, ulong * limitsToReturn);
uint complementOperation(uint l, MREP * rep, ulong * pA, misBits * C, ulong* limits);
uint differenceOperation(uint l, MREP * repA, MREP * repB, ulong * pA, ulong * pB, misBits * C);
uint intersectionOperation(uint l, MREP * A, MREP * B, ulong * pA, ulong * pB, misBits * C);
uint symmetricDifferenceOperation(uint l, MREP * repA, MREP * repB, ulong * pA, ulong * pB, misBits * C);
ulong unionOperation(MREP * repA, MREP * repB, misBits * C);
ulong * posByLevel(MREP * X);
ulong posLastLevel(misBits * bitsMaped);


/*************************************************************************/

void SkipNodes(uint l, MREP * X, ulong * pX, ulong s){
	ulong newPos = pX[l]+s*K*K-1;
	uint nOnes=0;
	
	if(l < X->maxLevel){
		nOnes = rank(X->btl, newPos) - rank(X->btl, pX[l] - 1);
	}
	
	pX[l] = newPos + 1;
	
	if(nOnes > 0){
		SkipNodes(l+1, X, pX, nOnes);
	}
}

void fillin(int nlevel, misBits * C){
	ulong i, cant = 1;

	for(; nlevel<C->niveles; nlevel++){
		cant = cant*K*K;
		for(i = 0; i < cant; i++){
			setBit(C, nlevel, 1u);
		}
	}
}

void copy(uint l, MREP * rep, ulong * pX, ulong s, misBits * C){
	ulong end = pX[l] + s*K*K - 1;
	ulong nOnes = 0;

	if(l < rep->maxLevel){
		nOnes = rank(rep->btl, end) - rank(rep->btl, pX[l] - 1);
	}

	ulong i;

	for(i = pX[l]; i <= end; i++){
		setBit(C, l, isBitSet(rep->btl, i)?1u:0u);
	}

	pX[l] = end + 1;

	if(nOnes > 0){
		copy(l + 1, rep, pX, nOnes, C);
	}
}

void limitsCalculator(int cuadrante, ulong * limitsOriginal, ulong * limitsToReturn){
	//Sólo válido para K=2
	ulong medioCuadrante = (limitsOriginal[1] - limitsOriginal[0]) / 2;

	if(cuadrante == 0 || cuadrante == 2){
		limitsToReturn[0] = limitsOriginal[0];
		limitsToReturn[1] = limitsOriginal[0] + medioCuadrante;
	}else{
		limitsToReturn[0] = limitsOriginal[0] + medioCuadrante + 1;
		limitsToReturn[1] = limitsOriginal[1];
	}

	if(cuadrante == 0 || cuadrante == 1){
		limitsToReturn[2] = limitsOriginal[2];
		limitsToReturn[3] = limitsOriginal[2] + medioCuadrante;
	}else{
		limitsToReturn[2] = limitsOriginal[2] + medioCuadrante + 1;
		limitsToReturn[3] = limitsOriginal[3];
	}
}

void preFillin(int nlevel, misBits * C, ulong * limits, uint numNodes, uint maxLevel){
	int i, res;
	ulong lims[K*K];
	for(i=0; i<K*K; i++){
		res = 1u;
		limitsCalculator(i, limits, lims);
		if(nlevel < maxLevel){
			if(lims[1] < numNodes && lims[3] < numNodes){
				fillin(nlevel + 1, C);				
			}else if(lims[0] < numNodes && lims[2] < numNodes){
				preFillin(nlevel + 1, C, lims, numNodes, maxLevel);
			}else{
				res = 0u;
			}
		} else{
			if(lims[0] >= numNodes || lims[2] >= numNodes){
				res = 0u;		
			}
		}
		setBit(C, nlevel, res);
	}	
}

uint symmetricDifferenceOperation(uint l, MREP * repA, MREP * repB, ulong * pA, ulong * pB, misBits * C){
	uint writesomething = 0u;
	ulong* t = (ulong *) malloc(sizeof(ulong)*K*K);
	uint i;
	
	for(i = 0; i < K*K; i++){
		t[i] = 0u;
		if(isBitSet(repA->btl, pA[l]) && isBitSet(repB->btl, pB[l])){
			if(l < repA->maxLevel){
				t[i] = symmetricDifferenceOperation(l + 1, repA, repB, pA, pB, C) ? 1u : 0u;
			}		
		}else{
			if(isBitSet(repA->btl, pA[l])){
				if(l < repA->maxLevel){
					t[i] = 1u;
					copy(l + 1, repA, pA, 1u, C);
				}else{
					t[i] = 1u;
				}
			}else if (isBitSet(repB->btl, pB[l])){
				if(l < repB->maxLevel){
					t[i] = 1u;
					copy(l + 1, repB, pB, 1u, C);
				}else{
					t[i] = 1u;
				}
			}
		}
    	writesomething = (writesomething == 1u || t[i] == 1u) ? 1u : 0u;
		pA[l] = pA[l] + 1;
		pB[l] = pB[l] + 1;
	}

	if(writesomething == 1u || l == 0){
		for(i=0; i<K*K; i++){
			setBit(C, l, t[i]);
		}
	}

	return (writesomething==1u)?1u:0u;
}

uint differenceOperation(uint l, MREP * repA, MREP * repB, ulong * pA, ulong * pB, misBits * C){
	uint writesomething = 0u;
	ulong* t = (ulong *) malloc(sizeof(ulong)*K*K);
	uint i;
	
	for(i = 0; i < K*K; i++){
		t[i] = 0u;
		if(isBitSet(repA->btl, pA[l]) && isBitSet(repB->btl, pB[l])){
			if(l < repA->maxLevel){
				t[i] = differenceOperation(l + 1, repA, repB, pA, pB, C) ? 1u : 0u;
			}else{
				t[i] = (isBitSet(repA->btl, pA[l]) && !isBitSet(repB->btl, pB[l])) ? 1u : 0u;		
			}				
		}else{
			if(isBitSet(repA->btl, pA[l]) && !isBitSet(repB->btl, pB[l])){
				if(l < repA->maxLevel){
					t[i] = 1u;
					copy(l + 1, repA, pA, 1u, C);
				}else{
					t[i] = 1u;
				}
			}
			else if(pB[l] && isBitSet(repB->btl, pB[l])){
				SkipNodes(l + 1, repB, pB, 1u);
			}
		}
    	writesomething = (writesomething == 1u || t[i] == 1u) ? 1u : 0u;
		pA[l] = pA[l] + 1;
		pB[l] = pB[l] + 1;
	}

	if(writesomething == 1u || l == 0){
		for(i=0; i<K*K; i++){
			setBit(C, l, t[i]);
		}
	}

	return (writesomething==1u)?1u:0u;
}

uint complementOperation(uint l, MREP * rep, ulong * pA, misBits * C, ulong* limits){
	uint writesomething = 0u;
	uint t[K*K];
	ulong lims[4];
	uint i;

	for(i = 0; i < K*K; i++){
		t[i] = 1u;
		limitsCalculator(i, limits, lims);
		if(isBitSet(rep->btl, pA[l])){
			if(l < rep->maxLevel && lims[0] < rep->numberOfNodes && lims[2] < rep->numberOfNodes){
				t[i] = complementOperation(l + 1, rep, pA, C, lims);
			}else{
				t[i] = 0u;
			}				
		}else{
			if(l < rep->maxLevel && lims[0] < rep->numberOfNodes && lims[2] < rep->numberOfNodes) {
				// Ya no se llama directamente a fillin sino que se generó una función
				// que verifica los límites previo al llamado de fillin
				preFillin(l+1, C, lims, rep->numberOfNodes, rep->maxLevel);
			} else if(lims[1] >= rep->numberOfNodes || lims[3] >= rep->numberOfNodes){
				t[i] = 0u;
			}
		}
        writesomething = (writesomething == 1u || t[i] == 1u) ? 1u : 0u;
		pA[l] = pA[l] + 1;
    }

	if(writesomething == 1u || l == 0){
		for(i=0; i<K*K; i++){
			setBit(C, l, t[i]);
		}
	}

	return (writesomething==1u)?1u:0u;
}


uint intersectionOperation(uint l, MREP * A, MREP * B, ulong * pA, ulong * pB, misBits * C){
	uint writeSomething = 0u;
	uint i;
	uint maxH = (A->maxLevel > B->maxLevel) ? A->maxLevel : B->maxLevel;
	ulong t[K*K];

	for(i=0; i<K*K; i++){
		if(l<maxH){
			if(isBitSet(A->btl, pA[l]) && isBitSet(B->btl, pB[l])){
				t[i] = intersectionOperation(l+1, A, B, pA, pB, C);
			}
			else{
				t[i] = 0u;
				if(isBitSet(A->btl, pA[l])){
					SkipNodes(l+1, A, pA, 1u);
				}
				if(isBitSet(B->btl, pB[l])){
					SkipNodes(l+1, B, pB, 1u);
				}
			}
		}
		else{
			t[i] = (isBitSet(A->btl, pA[l]) && isBitSet(B->btl, pB[l]))?1u:0u;
		}
		writeSomething = (writeSomething==1u || t[i]==1u)?1u:0u;
		pA[l] = pA[l]+1;
		pB[l] = pB[l]+1;
	}

	if(writeSomething==1u || l == 0){
		for(i=0; i<K*K; i++){
			setBit(C, l, t[i]);
		}		
	}

	return (writeSomething==1u)?1u:0u;
}

ulong unionOperation(MREP * repA, MREP * repB, misBits * C){
	//Tamaño para los NodeEntry de la Cola.
	unsigned char NodeTam = 100;
	QUEUE *q = newQueue(NodeTam);
	//ENTRY es el tipo de dato que almacena la tripleta
	Insert(q,setEntry(0,1,1));
	ulong pA, pB, vinculos = 0;
	pA = 0;
	pB = 0;
	ENTRY e;
	uint i, bA, bB;

	while(q->cant > 0){
//		printf("pA: %d - pB: %d - inserciones: %d\n", pA, pB, inserciones);
		e = Delete(q);
		for(i = 0; i<K*K; i++){
			bA = 0;
			bB = 0;
			if(getRa(e) == 1){
				bA = isBitSet(repA->btl, pA)?1:0;
				pA = pA + 1;
			}
			if(getRb(e) == 1){
				bB = isBitSet(repB->btl, pB)?1:0;
				pB = pB + 1;
			}
			if(bA || bB){
				setBit(C, 0, 1u);
				if(getLevel(e) < repA->maxLevel || getLevel(e) < repB->maxLevel){
					Insert(q,setEntry(getLevel(e)+1,bA,bB));
				}else{
					vinculos++;
				}
			}		
			else{
				setBit(C, 0, 0u);
			}
		}
	}
	destroyNodeQueue(q->front);
	free(q);
	return vinculos;
}

/*************************************************************************/
/***********************   Otros Algoritmos   ****************************/
/*************************************************************************/

ulong * posByLevel(MREP * X){
	//Calcula las posiciones del bitmap donde comienza cada nivel de X
	ulong * pX = (ulong *) malloc(sizeof(ulong) * (X->maxLevel + 1));
	if(pX == NULL){
		printf("Error en al reserva de memoria.\n");
		return NULL;
	}
	pX[0]=0;
	if(X->maxLevel>=1){
		pX[1]=K*K;
	}	
	uint i = 0;
	for(i=2; i<= X->maxLevel; i++){
		pX[i] = rank(X->btl, pX[i-1]-1) * (K*K) + (K*K);
	}
	return pX;
}

ulong posLastLevel(misBits * bitsMaped/*, uint maxLevel*/){
	//Calcula la posición en la que inicia el último nivel del bitmap

	ulong posBase=0, acumulador=1, tope=0, i; 
	while(posBase+tope < bitsMaped->cant && acumulador != 0){
		posBase += tope;
		tope = acumulador*K*K;
		acumulador = 0;
		for(i=0; i<tope; i++){
			if(isBitSeted(bitsMaped, 0u, posBase+i)){
				acumulador ++;
			}
		}
	}
	return posBase;
}

MREP * createFromBitmap(misBits * C, int maximalLevel, ulong numNodos, ulong numEdges){
	// numEdges podría ser -1 en el caso de no conocer el número de vínculos. 
	// Esto para el caso de la unión donde no se puede determinar el número de vínculos previamente.
	MREP * result;
	result = (MREP *) malloc(sizeof(struct matrixRep));
	if(result == NULL){
		printf("Problema al reservar memoria del MREP.\n");
		return NULL;
	}

	result->maxLevel = maximalLevel;
	result->btl_len = C->cant;
	ulong posUltimoNivel = posLastLevel(C);

	result->btl = createBitRankW32Int(C->bitsm, posUltimoNivel, 1, 20);
	result->bt_len = posUltimoNivel;

	free(C->pos);
	free(C->n);
	free(C);

	result->numberOfNodes = numNodos;
	result->numberOfEdges = numEdges;
	result->numberOfMarkedEdges = 0;
	result->info =NULL;
	result->element = NULL;	
	result->basex = NULL;
	result->basey = NULL;
	result->iniq = -1;
	result->finq = -1;
	result->info2[0] = NULL;
	result->info2[1] = NULL;
	result->div_level_table = NULL;

	return result;
}

MREP * k2tree_union(MREP * repA, MREP * repB){
	if(repA == NULL || repB == NULL){
		printf("Fallo en la operación por parámetro NULL.\n");
		return NULL;
	}

	uint maximalLevel = repA->maxLevel;

	ulong i = 0, maximalBits = 0, calculador = 1; 
	for(i = 0; i <= maximalLevel; i++){
		calculador *= (K*K);
		maximalBits += calculador;
	}
	maximalBits = maximalBits < (repA->btl_len + repB->btl_len) ? maximalBits : repA->btl_len + repB->btl_len;
	ulong minBits[] = {maximalBits};
	// En la unión, el peor de los casos corresponde a generar una 
	// representación completa de un k2-tree, sin sectores en 0.
	misBits * C = nuevoBitMap(1u, minBits);
	if(C == NULL){
		printf("Error en la reserva de memoria del bitmap.\n");
		return NULL;
	}

	ulong numEdges = unionOperation(repA, repB, C);
	concatenar(C);
	ulong numNodos = repA->numberOfNodes;
	return createFromBitmap(C, maximalLevel, numNodos, numEdges);
}

MREP * k2tree_symmetricdifference(MREP * repA, MREP * repB){
	uint maximalLevel = repA->maxLevel;	
	ulong * pRepA = posByLevel(repA);
	ulong * pRepB = posByLevel(repB);
	ulong * minBits = (ulong *) malloc(sizeof(ulong)*(maximalLevel+1));

	if(minBits == NULL){
		printf("Error en la reserva de memoria.\n");
		return NULL;
	}
	// En el peor de los casos (en términos de espacio), la diferencia simétrica podría 
	// generar una representación de la suma de ambos elementos, 
	// ya que podrían no existir elementos en común entre ellos
	uint i;
	for(i=0; i<=maximalLevel; i++){
		minBits[i] = pRepA[i+1] - pRepA[i] + pRepB[i+1] - pRepB[i];
	}
	minBits[maximalLevel] = repA->btl_len - pRepA[maximalLevel] + repB->btl_len - pRepB[maximalLevel];

/*
	uint i;
	ulong auxNodes = K*K;

	for(i=0; i<=maximalLevel; i++){
		minBits[i] = auxNodes;
		//maximalNodes+=minBits[i];
		auxNodes = auxNodes * (K*K);
	}
*/
	misBits * C = nuevoBitMap(maximalLevel+1, minBits);	

	if(C == NULL){
		printf("Error en la reserva de memoria.\n");
		return NULL;
	}

	symmetricDifferenceOperation(0u, repA, repB, pRepA, pRepB, C);
	ulong vinculolos = concatenar(C);	
	ulong numNodos = repA->numberOfNodes;

	return createFromBitmap(C, maximalLevel, numNodos, vinculolos);
}

MREP * k2tree_intersection(MREP * repA, MREP * repB){
	uint maximalLevel = repA->maxLevel;	
	ulong * pRepA = posByLevel(repA);
	ulong * pRepB = posByLevel(repB);	
	ulong * minBits = (ulong *) malloc(sizeof(ulong)*(maximalLevel+1));
	
	if(minBits == NULL){
		printf("Se ha generado un problema...\n");
		return NULL;
	}

	// La intersección podría tener a lo más, tantas celdas activas 
	// como las que posee el menor de los dos elementos operados.

	ulong numBitsByLevelA, numBitsByLevelB;
	uint i;

	for(i = 0; i <= maximalLevel; i++){
		if(i==repA->maxLevel){
			numBitsByLevelA = repA->btl_len - pRepA[i];
			numBitsByLevelB = repB->btl_len - pRepB[i];
		}else{
			numBitsByLevelA = pRepA[i+1] - pRepA[i];
			numBitsByLevelB = pRepB[i+1] - pRepB[i];
		}
		minBits[i] = (numBitsByLevelA > numBitsByLevelB)?numBitsByLevelB:numBitsByLevelA;
	}

	misBits * C = nuevoBitMap(maximalLevel+1, minBits);

	if(C == NULL){
		printf("Error en la reserva de memoria.\n");
		return NULL;
	}

	intersectionOperation(0u, repA, repB, pRepA, pRepB, C);
	ulong vinculolos = concatenar(C);	
	ulong numNodos = repA->numberOfNodes;

	return createFromBitmap(C, maximalLevel, numNodos, vinculolos);
}

MREP * k2tree_difference(MREP * repA, MREP * repB){
	ulong maximalBits = repA->btl_len;
	uint maximalLevel = repA->maxLevel;
	
	ulong * pRepA = posByLevel(repA);
	ulong * pRepB = posByLevel(repB);
	
	ulong * minBits = (ulong *) malloc(sizeof(ulong)*(maximalLevel+1));
	if(minBits == NULL){
		printf("Error en la reserva de memoria.\n");
		return NULL;
	}
	// En la diferencia, la reserva de espacio debe ser suficiente para almacenar
	// la representación completa del primer k2-tree, ya que podrían no tener
	// elementos en común.
	uint i;
	for(i=0; i<=maximalLevel; i++){
		minBits[i] = pRepA[i+1] - pRepA[i];
	}
	minBits[maximalLevel] = maximalBits - pRepA[maximalLevel];

	misBits * C = nuevoBitMap(maximalLevel+1, minBits);	
	if(C == NULL){
		printf("Error en la reserva de memoria.\n");
		return NULL;
	}
	
	differenceOperation(0u, repA, repB, pRepA, pRepB, C);
	ulong vinculolos = concatenar(C);	
	ulong numNodos = repA->numberOfNodes;
	return createFromBitmap(C, maximalLevel, numNodos, vinculolos);
}

MREP * k2tree_complement(MREP * repA){
	uint maximalLevel = repA->maxLevel;

	ulong * pRepA = posByLevel(repA);

	ulong * minBits = (ulong *) malloc(sizeof(ulong) * (maximalLevel+1));
	if(minBits == NULL){
		printf("Error en la reserva de memoria.\n");
		return NULL;
	}
	// Para el complemento, el peor caso es generar como resultado de la
	// operación una representación que no compacte sectores
	// Por lo que se reserva el espacio necesario para una representación completa
	
	ulong i, auxNodes = K*K;
	for(i=0; i<=maximalLevel; i++){
		minBits[i] = auxNodes;
		//maximalNodes+=minBits[i];
		auxNodes = auxNodes * (K*K);
	}
	
	misBits * C = nuevoBitMap(maximalLevel+1, minBits);
	if(C == NULL){
		printf("Error en la reserva de memoria.\n");
		return NULL;
	}

//	Variable para calcular el número de vínculos dentro de la operación.
	ulong limiteSuperior = repA->div_level_table[0] * 2 - 1;
	ulong limits[4] = {0, limiteSuperior, 0, limiteSuperior};

	complementOperation(0u, repA, pRepA, C, limits);
	ulong vinculolos = concatenar(C);
	ulong numNodos = repA->numberOfNodes;
	return createFromBitmap(C, maximalLevel, numNodos, vinculolos);
}
