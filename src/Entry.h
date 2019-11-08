#include <stdio.h>
typedef struct entry 
 {
 /****************************
 * b b b b b b  b  b         *
 * -----l-----  Ra Rb        *
 *****************************
 */ 
 unsigned char e; 
 }ENTRY;

/*
* Crea un objeto de tipo ENTRY
*/
ENTRY setEntry(unsigned char l, unsigned char ra, unsigned char rb);
/*
* obtiene el nivel 
*/
unsigned char  getLevel(ENTRY e);
/*
* obtiene Ra 
*/
unsigned char  getRa(ENTRY e);
/*
* obtiene Rb 
*/
unsigned char getRb(ENTRY e);
