#ifndef __ENCODING_H__
#define __ENCODING_H__
#include "basic.h"


uint getEncodedSize(uint in);

uint encodeValue(uint in, byte *out);

uint decodeValue(byte *encoded, uint *bytesRead);

uint locateEntry(byte * encoded, uint byteSize, uint nEntry);

uint countEntries(byte *vbyte, uint byteSize);


#endif

