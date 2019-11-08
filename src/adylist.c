#include <math.h>
#include "misBits.h"
#include "adylist.h"

ALREP * loadAdyacencyList(char * basename){
	// Apertura del archivo
	char *filename = (char *) malloc(sizeof(char)*(strlen(basename)+8));
	strcpy(filename,basename);
	strcat(filename,".rbfull");
	FILE * ft = fopen(filename,"r");
	// Declaración de estructura
	ALREP * list;
	list = (ALREP *) malloc(sizeof(struct adyList));
	// Lectura desde el archivo
	fread(&list->numNodes,sizeof(uint),1,ft);
	fread(&list->numEdges,sizeof(ulong),1,ft);
	// Reserva de la lista de adyacencia
	list->listady = (int*)malloc(sizeof(int)*(list->numNodes+list->numEdges));
	fread(list->listady,sizeof(int), list->numNodes+list->numEdges,ft);
	fclose(ft);
	free(filename);
	return list;
}

void destroyAdyacencyList(ALREP * list){
	if(list->listady!=NULL){
		free(list->listady);
	}
	free(list);
	list = NULL;
}

void saveAdyacencyList(ALREP * list, char * basename){
	char *filename = (char *)malloc(sizeof(char)*(strlen(basename)+8));
	strcpy(filename,basename);
	strcat(filename,".rbfull");
	FILE *fr = fopen(filename,"w");
	fwrite(&list->numNodes,sizeof(uint),1,fr);
	fwrite(&list->numEdges,sizeof(ulong),1,fr);

	fwrite(list->listady,sizeof(int),list->numNodes+list->numEdges,fr);
	fclose(fr);
	free(filename);
}

uint adylist_getCell(ALREP * lista, uint p0, uint p1){
	// Retorna 1 si existe la relación desde p0 hacia p1
	// 0 en otro caso
	if(p0 > lista->numNodes || p1 > lista->numNodes || p0 * p1 == 0){
		return 0;
	}
	
	int nodoP = -1 * p0;
	uint i = 0;	
	while(nodoP != lista->listady[i]){
		i++;
	}	
	do{
		i++;
	}while(lista->listady[i] > 0 && lista->listady[i] != p1);
	
	return (p1 == lista->listady[i]) ? 1 : 0;
}


uint * adylist_getNeightbors(ALREP * lista, uint p0){
	// Retorna una lista de enteros sin signo con n+1 elementos
	// donde el primer entero indica la cantidad de vecinos de p0
	// y los demás n enteros corresponden a los IDs de los vecinos
	if(p0 < 1 || p0 > lista->numNodes){
		printf("ID de nodo no válido. Rango entre 1 y %u.\n", lista->numNodes);
		return NULL;
	}
	int nodoP = -1 * p0;
	uint i = 0;
	while(nodoP != lista->listady[i]){
		i++;
	}
	i++;
	uint pos0 = i;
	uint cont = 0;
	while(lista->listady[i++] > 0){
		cont++;
	}
	uint * neightbors = (uint *) malloc(sizeof(uint) * (cont + 1));
	neightbors[0] = cont;
	for(i= 0; i<cont; i++){
		neightbors[i+1] = lista->listady[i+pos0];
	}

	return neightbors;

}

uint * adylist_getRange(ALREP * lista, int p0, int p1, int q0, int q1){
	// Retorna una lista con todos los pares x,y donde se cumple
	// p0 <= x <= p1 && q0 <= y <= q1
	// En la lista se indica, por cada nodo del eje x contenido en el rango
	// un entero i que indica la cantidad de vínculos que posee ese nodo
	// seguido de los i enteros identificando el nodo de destino de la relación
	uint nodos = lista->numNodes;
	if(p0 * p1 * q0 * q1 == 0 || p0 > nodos || p1 > nodos || q0 > nodos || q1 > nodos){
		printf("ID de nodo no válido. Rango entre 1 y %u.\n", lista->numNodes);
		return NULL;
	}

	if(p0 > p1 || q0 > q1){
		printf("Se debe cumplir que ID_0 < = ID_1, para p y q.\n");
		return NULL;
	}

	uint i = 0u, tope;				// Para el recorrido de << lista >>
	int j;
	uint posAct = 1, posAnt = 0u;	// Para el recorrido de << retorno >>
	tope = (uint) (lista->numNodes + lista->numEdges);

	uint capacidad = (p1 - p0 + 1) * (q1 - q0 + 2);
	uint * retorno = (uint *) malloc(sizeof(uint) * capacidad);
	if(retorno == NULL){
		printf("Error en la solicitud de memoria.\n");
		return NULL;
	}
	for(j=p0 * -1; j >= p1 * -1; j--){
		// Recorrido hasta el siguiente p
		while(i < tope && lista->listady[i] != j){
			i++;
		}

		// Recorrido hasta el rango q
		do{
			i++;
		}while(i < tope && lista->listady[i] < q0 && lista->listady[i] > 0);

		// Insertando los valores correspondientes si los hay
		while(i < tope && lista->listady[i] >= q0 && lista->listady[i] <= q1){
			retorno[posAct] = lista->listady[i];
			posAct++;
			i++;
		}
		// Insertando indicador de cantidad por cada sub-lista
		retorno[posAnt] = posAct - posAnt - 1;
		posAnt = posAct;
		posAct++;		
	}
	return retorno;
}

uint * adylist_getReverseNeightbors(ALREP * lista, uint p0){
	// Retorna una lista de enteros sin signo con n+1 elementos
	// donde el primer entero indica la cantidad de vecinos reversos de p0
	// y los demás n enteros corresponden a los IDs de los vecinos reversos
	if(p0 < 1 || p0 > lista->numNodes){
		printf("ID de nodo no válido. Rango entre 1 y %u.\n", lista->numNodes);
		return NULL;
	}
	int nodoP = 0;
	uint i, contador = 0;
	for(i=0; i < lista->numNodes + lista->numEdges; i++){
		if(lista->listady[i] == p0){
			contador++;
		}
	}
	
	uint * revNeightbors = (uint *) malloc(sizeof(uint) * (contador + 1));
	uint pos = 0;
	revNeightbors[pos++] = contador;
	for(i=0; i < lista->numNodes + lista->numEdges; i++){
		if(lista->listady[i] < 0){
			nodoP = lista->listady[i] * -1;
		}else if(lista->listady[i] == p0){
			revNeightbors[pos++] = nodoP;
		}
	}

	return revNeightbors;
}

ulong * indexList(ALREP * list){
	if(list == NULL){
		return NULL;
	}
	ulong posList = 0, posIndex = 0;
	ulong tope = list->numNodes + list->numEdges;
	ulong * index;
	index = (ulong *) malloc(sizeof(ulong) * list->numNodes);

	while(posList < tope && posIndex < list->numNodes){
		if(list->listady[posList] < 0){
			index[posIndex++] = posList;
		}
		posList++;
	}

	return index;
}