#include "Entry.h"

inline unsigned char getLevel(ENTRY e) { 
	return e.e >>2;
}

inline unsigned char getRa(ENTRY e) { 
	return (e.e & 0x02)>>1;
}

inline unsigned char getRb(ENTRY e) { 
	return e.e & 0x01;
}

inline ENTRY setEntry(unsigned char l, unsigned char ra, unsigned char rb) { 
	ENTRY e ; 
	e.e = (unsigned char) (l<<2)  + (ra<<1) + (rb);
	return e;
}
