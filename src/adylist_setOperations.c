#include <stdio.h>
#include "adylist_setOperations.h"

/*
	Asumiendo que se opera sobre dos grafos con los mismos nodos.
*/
ALREP * adylistUnionOperation(ALREP * A, ALREP * B){
	ulong edgesAprox;
	ulong posA=0, posB=0, posResult = 0;

	if(A->numNodes != B->numNodes){
		printf("Las listas deben tener el mismo numero de nodos (%u <> %u).\n",A->numNodes, B->numNodes);
		return NULL;
	}

//	Se reservará la menor cantidad de espacio teniendo en cuenta que el resultado de
//	la unión de ambas listas podría ser a lo más el tamaño de ambas siempre que éste sea
//	menor que definir todos vínculos posibles
//	Se debe reservar espacio para los links (edges) y los nodos (nodes)

	ulong maximalNodes = A->numNodes*A->numNodes+A->numNodes;
	ulong idealLength = A->numEdges + B->numEdges + A->numNodes;
	edgesAprox = idealLength < maximalNodes ? idealLength : maximalNodes;
	
	ALREP * result;
	result = (ALREP *) malloc(sizeof(struct adyList));
	result->listady = (int *) malloc(sizeof(int)*(edgesAprox));
	if(result == NULL){
		printf("Error al reservar memoria.\n");
		return NULL;
	}
	// Operación de Unión
	while(posA < A->numEdges + A->numNodes && posB < B->numEdges + B->numNodes){
		if(A->listady[posA] < 0 && B->listady[posB] < 0){
			//Ambas listas llegaron a un nuevo nodo
			result->listady[posResult] = A->listady[posA];
			posResult++;
			posA++;
			posB++;
		}else if(A->listady[posA] < 0){
			// La lista A llegó a un nuevo nodo, se deben añadir los elementos restantes de la 
			// lista B en el resultado
			while(posB < B->numEdges + B->numNodes && B->listady[posB] > 0){
				result->listady[posResult] = B->listady[posB];
				posResult++;
				posB++;
			}
		}else if(B->listady[posB] < 0){
			// La lista B llegó a un nuevo nodo, se deben añadir los elementos restantes de la 
			// lista A en el resultado
			while(posA < A->numEdges + A->numNodes && A->listady[posA] > 0){
				result->listady[posResult] = A->listady[posA];
				posResult++;
				posA++;
			}
		}else{
			// Se están analizando los vínculos del mismo nodo en ambas listas
			if(A->listady[posA] < B->listady[posB]){
				result->listady[posResult] = A->listady[posA];
				posResult++;
				posA++;
			}else if(A->listady[posA] > B->listady[posB]){
				result->listady[posResult] = B->listady[posB];
				posResult++;
				posB++;
			}else{
				result->listady[posResult] = A->listady[posA];
				posResult++;
				posA++;
				posB++;
			}
		}
	}
	// Una de las listas de adyacencia puede aún contener elementos sin procesar
	// del último nodo
	while(posA < A->numEdges + A->numNodes){
		result->listady[posResult] = A->listady[posA];
		posResult++;
		posA++;
	}
	while(posB < B->numEdges + B->numNodes){
		result->listady[posResult] = B->listady[posB];
		posResult++;
		posB++;
	}

	// Se redimensiona el arreglo reservado para el resultado.
	if(posResult < edgesAprox){
		result->listady = (int *) realloc(result->listady, sizeof(int)*(posResult));
	}

	// Finalización
	result->numEdges = posResult - A->numNodes;
	result->numNodes = A->numNodes;
	return result;
}

ALREP * adylistDifferenceOperation(ALREP * A, ALREP * B){
	ulong edgesAprox;
	ulong posA=0, posB=0, posResult = 0;

	if(A->numNodes != B->numNodes){
		printf("Las listas deben tener el mismo numero de nodos.\n");
		return NULL;
	}

//	Se reservará la menor cantidad de espacio teniendo en cuenta que el resultado de
//	la diferencia podría ser a lo más el tamaño de A
//	Se debe reservar espacio para los links (edges) y los nodos (nodes)

	edgesAprox = A->numEdges + A->numNodes;
	ALREP * result;
	result = (ALREP *) malloc(sizeof(struct adyList));
	result->listady = (int *) malloc(sizeof(int)*(edgesAprox));

	if(result == NULL){
		printf("Error al reservar memoria.\n");
		return NULL;
	}
	// Operación de Diferencia
	while(posA < A->numEdges + A->numNodes && posB < B->numEdges + B->numNodes){
		if(A->listady[posA] < 0 && B->listady[posB] < 0){
			//Ambas listas llegaron a un nuevo nodo
			result->listady[posResult] = A->listady[posA];
			posResult++;
			posA++;
			posB++;
		}else if(A->listady[posA] < 0){
			// La lista A llegó a un nuevo nodo, se debe alcanzar el mismo nodo en la lista B
			// descartando alguna nueva insersión en el resultado
			while(posB < B->numEdges + B->numNodes && B->listady[posB] > 0){
				posB++;
			}
		}else if(B->listady[posB] < 0){
			// La lista B llegó a un nuevo nodo, se deben añadir los elementos restantes de la 
			// lista A en el resultado
			while(posA < A->numEdges + A->numNodes && A->listady[posA] > 0){
				result->listady[posResult] = A->listady[posA];
				posResult++;
				posA++;
			}
		}else{
			// Se están analizando los vínculos del mismo nodo en ambas listas
			if(A->listady[posA] < B->listady[posB]){
				result->listady[posResult] = A->listady[posA];
				posResult++;
				posA++;
			}else if(A->listady[posA] > B->listady[posB]){
				// El vínculo de B es menor y se debe, al menos, alcanzar A
				while(posB < B->numEdges + A->numNodes && B->listady[posB] > 0 && A->listady[posA] > B->listady[posB]){
					posB++;					
				}
			}else{
				// Los vínculos apuntan al mismo nodo por lo que se descarta del resultado
				posA++;
				posB++;
			}
		}
	}
	// La lista de adyacencia A puede aún contener elementos sin procesar del último nodo
	while(posA < A->numEdges + A->numNodes){
		result->listady[posResult] = A->listady[posA];
		posResult++;
		posA++;
	}

	// Se redimensiona el arreglo reservado para el resultado.
	if(posResult < edgesAprox){
		result->listady = (int *) realloc(result->listady, sizeof(int)*(posResult));
	}

	// Finalización
	result->numEdges = posResult-A->numNodes;
	result->numNodes = A->numNodes;
	return result;
}

ALREP * adylistIntersectionOperation(ALREP * A, ALREP * B){
	ulong edgesAprox;
	ulong posA=0, posB=0, posResult = 0;

	if(A->numNodes != B->numNodes){
		printf("Las listas deben tener el mismo numero de nodos.\n");
		return NULL;
	}

//	Se reservará la menor cantidad de espacio teniendo en cuenta que el resultado de
//	la Intersección podría ser a lo más el tamaño de la menor de las listas
//	Se debe reservar espacio para los links (edges) y los nodos (nodes)

	edgesAprox = (A->numEdges <= B->numEdges ? A->numEdges : B->numEdges) + A->numNodes;
	ALREP * result;
	result = (ALREP *) malloc(sizeof(struct adyList));
	result->listady = (int *) malloc(sizeof(int)*(edgesAprox));

	if(result == NULL){
		printf("Error al reservar memoria.\n");
		return NULL;
	}

	// Operación de Intersección
	while(posA < A->numEdges + A->numNodes && posB < B->numEdges + B->numNodes){
		if(A->listady[posA] < 0 && B->listady[posB] < 0){
			//Ambas listas llegaron a un nuevo nodo
			result->listady[posResult] = A->listady[posA];
			posResult++;
			posA++;
			posB++;
		}else if(A->listady[posA] < 0){
			// La lista A llegó a un nuevo nodo, se debe alcanzar el mismo nodo en la lista B
			// descartando alguna nueva insersión en el resultado
			while(posB < B->numEdges + B->numNodes && B->listady[posB] > 0){
				posB++;
			}
		}else if(B->listady[posB] < 0){
			// La lista B llegó a un nuevo nodo, se debe alcanzar el mismo nodo en la lista A
			// descartando alguna nueva insersión en el resultado
			while(posA < A->numEdges + A->numNodes && A->listady[posA] > 0){
				posA++;
			}
		}else{
			// Se están analizando los vínculos del mismo nodo en ambas listas
			if(A->listady[posA] < B->listady[posB]){
				// El vínculo de A es menor y se debe, al menos, alcanzar B
				while(posA < A->numEdges + A->numNodes && A->listady[posA] > 0 && A->listady[posA] < B->listady[posB]){
					posA++;					
				}
			}else if(A->listady[posA] > B->listady[posB]){
				// El vínculo de B es menor y se debe, al menos, alcanzar A
				while(posB < B->numEdges + B->numNodes && B->listady[posB] > 0 && A->listady[posA] > B->listady[posB]){
					posB++;					
				}
			}else{
				// Los vínculos apuntan al mismo nodo por lo que se considera en el resultado
				result->listady[posResult] = A->listady[posA];
				posResult++;
				posA++;
				posB++;
			}
		}
	}

	// No importa si alguna de las listas quedó con nodos sin verificar, se descartan 
	// por no existir intersección.

	// Se redimensiona el arreglo reservado para el resultado.
	if(posResult < edgesAprox){
		result->listady = (int *) realloc(result->listady, sizeof(int)*(posResult));
	}

	// Finalización
	result->numEdges = posResult-A->numNodes;
	result->numNodes = A->numNodes;
	return result;	
}

ALREP * adylistSymmetricDifferenceOperation(ALREP * A, ALREP * B){
	ulong edgesAprox;
	ulong posA=0, posB=0, posResult = 0;

	if(A->numNodes != B->numNodes){
		printf("Las listas deben tener el mismo numero de nodos.\n");
		return NULL;
	}

//	Se reservará la menor cantidad de espacio teniendo en cuenta que el resultado de
//	la diferencia simétrica podría ser a lo más el tamaño de ambas siempre que éste sea
//	menor que definir todos vínculos posibles
//	Se debe reservar espacio para los links (edges) y los nodos (nodes)
	ulong maximalNodes = A->numNodes*A->numNodes+A->numNodes;
	ulong idealLength = A->numEdges + B->numEdges + A->numNodes;
	edgesAprox = idealLength < maximalNodes ? idealLength : maximalNodes;
	
	ALREP * result;
	result = (ALREP *) malloc(sizeof(struct adyList));
	result->listady = (int *) malloc(sizeof(int)*(edgesAprox));
	if(result == NULL){
		printf("Error al reservar memoria.\n");
		return NULL;
	}

	// Operación de Diferencia Simétrica
	while(posA < A->numEdges + A->numNodes && posB < B->numEdges + B->numNodes){
		if(A->listady[posA] < 0 && B->listady[posB] < 0){
			//Ambas listas llegaron a un nuevo nodo
			result->listady[posResult] = A->listady[posA];
			posResult++;
			posA++;
			posB++;
		}else if(A->listady[posA] < 0){
			// La lista A llegó a un nuevo nodo, se debe alcanzar el mismo nodo 
			// insertando todos los enlaces encontrados desde la lista B
			while(posB < B->numEdges + B->numNodes && B->listady[posB] > 0){
				result->listady[posResult] = B->listady[posB];
				posResult++;
				posB++;
			}
		}else if(B->listady[posB] < 0){
			// La lista B llegó a un nuevo nodo, se debe alcanzar el mismo nodo 
			// insertando todos los enlaces encontrados desde la lista A
			while(posA < A->numEdges + A->numNodes && A->listady[posA] > 0){
				result->listady[posResult] = A->listady[posA];
				posResult++;
				posA++;
			}
		}else{
			// Se están analizando los vínculos del mismo nodo en ambas listas
			if(A->listady[posA] < B->listady[posB]){
				// El enlace de A no está en B, por lo que se debe añadir al resultado
				result->listady[posResult] = A->listady[posA];
				posResult++;
				posA++;
			}else if(A->listady[posA] > B->listady[posB]){
				// El enlace de B no está en A, por lo que se debe añadir al resultado
				result->listady[posResult] = B->listady[posB];
				posResult++;
				posB++;
			}else{
				// Los vínculos apuntan al mismo nodo por lo que se descarta del resultado
				posA++;
				posB++;
			}
		}
	}
	// La lista de adyacencia A puede aún contener elementos sin procesar del último nodo
	// que se deben añadir al resultado
	while(posA < A->numEdges + A->numNodes){
		result->listady[posResult] = A->listady[posA];
		posResult++;
		posA++;
	}
	// La lista de adyacencia B puede aún contener elementos sin procesar del último nodo
	// que se deben añadir al resultado
	while(posB < B->numEdges + B->numNodes){
		result->listady[posResult] = B->listady[posB];
		posResult++;
		posB++;
	}

	// Se redimensiona el arreglo reservado para el resultado.
	if(posResult < edgesAprox){
		result->listady = (int *) realloc(result->listady, sizeof(int)*(posResult));
	}

	// Finalización
	result->numEdges = posResult - A->numNodes;
	result->numNodes = A->numNodes;
	return result;
}

ALREP * adylistComplementOperation(ALREP * A){
	ulong edgesAprox;
	ulong posA=0, posResult = 0;
	int anterior=0, i=0;

//	Se reservará la cantidad exacta de espacio teniendo en cuenta que el resultado del
//	complemento será el total de vínculos posibles menos los vínculos de A
//	Se debe reservar espacio para los links (edges) y los nodos (nodes)

	edgesAprox = A->numNodes * A->numNodes - A->numEdges + A->numNodes;
	ALREP * result;
	result = (ALREP *) malloc(sizeof(struct adyList));
	result->listady = (int *) malloc(sizeof(int)*(edgesAprox));
	if(result == NULL){
		printf("Error al reservar memoria.\n");
		return NULL;
	}

	// Operación de Complemento
	while(posA < A->numEdges + A->numNodes){
		if(A->listady[posA] < 0){
		
		//	Se comienza un nuevo nodo, pero antes de añadirlo a result se debe 
		//	comprobar que el nodo anterior no haya estado vacío, o falten vínculos 
		//	que añadir al resultado.
		
			if(A->listady[posA] != -1){
				for(i = anterior + 1; i <= A->numNodes; i++){
					result->listady[posResult] = i;
					posResult++;
				}
			}
			result->listady[posResult] = A->listady[posA];
			posResult++;
			anterior = 0;
			posA++;
			// La variable anterior se marca en 0 para ser consistente con su tratamiento 
			// para el primer caso, donde si el primer vínculo fuese 1, el ciclo 
			// for no tendría efecto.
		}else{
			// Se deben incorporar al resultado todos los vínculos que están entre el anterior 
			// y el actual de A, sin incluirlos
			for(i = anterior + 1; i < A->listady[posA]; i++){
				result->listady[posResult] = i;
				posResult++;
			}
			// Una vez añadidos, se actualiza el valor de anterior
			anterior = A->listady[posA];
			posA++;
		}
	}
	// Se deben añadir los últimos elementos pendientes después del último vínculo en A
	for(i = anterior + 1; i <= A->numNodes; i++){
		result->listady[posResult] = i;
		posResult++;
	}

	// Finalización
	result->numEdges = posResult - A->numNodes;
	result->numNodes = A->numNodes;
	return result;
}