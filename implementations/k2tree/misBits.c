#include <stdio.h>
#include <stdlib.h>
#include "misBits.h"
#include "basic.h"

/*
	unsigned int * pos; // Posiciones de inicio para cada nivel del bitmap
	unsigned int * n; // Elementos de cada nivel del bitmap
	unsigned int niveles; // Cantidad de niveles del bitmap
	unsigned int tam; // Tamaño del bitmap
	unsigned int cant; // Elementos del bitmap
	unsigned int * bitsm;
*/

misBits* nuevoBitMap(uint levels, ulong * cants){

	misBits* bitses = (misBits *) malloc(sizeof(misBits));
	if(bitses == NULL){
		printf("Error en la reserva de memoria (Estructura misBits).\n");
		return NULL;
	}
	bitses->pos = (ulong*) malloc(sizeof(ulong)*levels);
	if(bitses->pos == NULL){
		printf("Error en la reserva de memoria (Posiciones misBits.\n");
		return NULL;
	}
	bitses->n = (ulong *) malloc(sizeof(ulong)*levels);
	if(bitses->n == NULL){
		printf("Error en la reserva de memoria (Niveles misBits).\n");
		return NULL;
	}
	bitses->niveles = levels;
	bitses->tam = 0;
	bitses->cant = 0;
	bitses->numEdges = 0;

	uint i;
	for(i=0; i< levels; i++){
		bitses->pos[i] = bitses->tam;
		bitses->n[i] = 0;
		bitses->tam += cants[i];
	}

	bitses->bitsm = (unsigned int *) malloc(sizeof(unsigned int) *(bitses->tam/W+1));
	if(bitses->bitsm == NULL){
		printf("Error en la reserva de memoria (Bits de misBits).\n");
		return NULL;
	}

	for(i=0; i<bitses->tam/W+1; i++){
		bitses->bitsm[i]=0;
	}
	return bitses;
}

void setBit(misBits* bitses, uint level, uint cont){
	if(bitses->tam == bitses->cant || level >= bitses->niveles){
		//Verifica que no esté lleno el bitmap
		// o que no se haya enviado como parámetro un nivel inexistente
		return;
	}
	ulong posicion = bitses->pos[level] + bitses->n[level];
	if( (level < bitses->niveles-1 && posicion >= bitses->pos[level+1])
		|| (level == bitses->niveles-1 && posicion >= bitses->tam) ){
		//Verifica que los segmentos no sobrepasen sus límites
		// (que no escriban en el siguiente nivel)
		return;
	}
	ulong change = bitses->bitsm[posicion/W];
	ulong posChange;
	if(cont==1){
		posChange = change | (1u << (posicion % W));
	}else{
		posChange = change & ~(1u << (posicion % W));
	}
	bitses->bitsm[posicion/W] = posChange;
	bitses->n[level]++;
	bitses->cant++;
}

void bitSeter(misBits* bitses, uint level, ulong i){
	if(bitses->tam == bitses->cant || level >= bitses->niveles){
		//Verifica que no esté lleno el bitmap
		// o que no se haya enviado como parámetro un nivel inexistente
		return;
	}
	ulong posicion = bitses->pos[level] + i;
	if( (level < bitses->niveles-1 && posicion >= bitses->pos[level+1])
		|| (level == bitses->niveles-1 && posicion >= bitses->tam) ){
		//Verifica que los segmentos no sobrepasen sus límites
		// (que no escriban en el siguiente nivel)
		return;
	}
	bitses->bitsm[posicion/W] = bitses->bitsm[posicion/W] | (1u << (posicion % W));
	return;
}

uint isBitSeted(misBits* bitses, uint level, ulong i){
	if(level >= bitses->niveles){
		return 0u;
	}
	ulong posicion = bitses->pos[level] + i;
	if( (level < bitses->niveles-1 && posicion >= bitses->pos[level+1])
		|| (level == bitses->niveles-1 && posicion >= bitses->tam) ){
		//Verifica que los segmentos no sobrepasen sus límites
		// (que no escriban en el siguiente nivel)
		return 0u;
	}
	if((1u << (posicion % W)) & bitses->bitsm[posicion/W]){
		return 1u;
	}
	return 0u;
}

void destruirBitMap(misBits* bitses){
	//free(bitses->bitsm);
	// bitsm se deja disponible para MREP * que se retorna de las operaciones
	if(bitses != NULL){
		if(bitses->pos != NULL){
			free(bitses->pos);
			bitses->pos = NULL;
		}
		if(bitses->n != NULL){
			free(bitses->n);
			bitses->n = NULL;
		}
		free(bitses);
	}
	bitses=NULL;
}

void destruirTodoBitmap(misBits* bitses){
	if(bitses != NULL && bitses->bitsm != NULL){
		free(bitses->bitsm);
		bitses->bitsm = NULL;
	}
	destruirBitMap(bitses);
}

ulong concatenar(misBits* bitses){
	ulong i, j, posicion;
	posicion = bitses->pos[0] + bitses->n[0];
	for(i=1; i < bitses->niveles; i++){
		for(j=0; j < bitses->n[i]; j++){
			if(isBitSeted(bitses, i, j)){
				bitses->bitsm[posicion/W] = bitses->bitsm[posicion/W] | (1u << (posicion % W));
			}else{
				bitses->bitsm[posicion/W] = bitses->bitsm[posicion/W] & ~(1u << (posicion % W));
			}
			posicion++;
		}
	}

	ulong auxPos = posicion;

	while((posicion-1)/W == posicion/W){
		bitses->bitsm[posicion/W] = bitses->bitsm[posicion/W] & ~(1u << (posicion % W));
		posicion++;
	}

	// Asegura que los bits finales de la representación sean 0.
	//for(i=posicion/W; i<bitses->tam/W+1 && i<posicion/W+8; i++){
	for(i=posicion/W; i<bitses->tam/W+1; i++){
		bitses->bitsm[i] = 0;
	}

	bitses->pos = (ulong*) realloc(bitses->pos, sizeof(ulong));
	bitses->n = (ulong*) realloc(bitses->n, sizeof(ulong));
	bitses->pos[0]=0;
	bitses->n[0] = auxPos;
	bitses->niveles = 1;
	bitses->bitsm = (uint *) realloc(bitses->bitsm, sizeof(uint) *(posicion/W));
	return bitses->numEdges;
}

void prepararBitmap(misBits* bitses, int objectsBRWT,uint numNodes){
	ulong i, j, counter;
	uint inNodeA, inNodeB;
	bitses->n[0] = objectsBRWT;
	bitses->n[1] = objectsBRWT;
	for(i=2; i<bitses->niveles; i+=2){
		counter = 0;
		bitses->n[i] = 0;
		for(j=0; j < numNodes; j++){
			inNodeA = isBitSeted(bitses, i, j);
			inNodeB = isBitSeted(bitses, i+1, j);
			if(inNodeA && inNodeB){
				setBit(bitses, i, 1u);
				setBit(bitses, i+1, 1u);
			}else if(inNodeA){
				setBit(bitses, i, 1u);
				setBit(bitses, i+1, 0u);
			}else if(inNodeB){
				setBit(bitses, i, 0u);
				setBit(bitses, i+1, 1u);
			}

			if(inNodeA || inNodeB){
				counter++;
			}
		}
		bitses->n[i] = counter;
		bitses->n[i+1] = counter;
	}
}