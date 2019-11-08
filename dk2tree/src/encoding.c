#include "encoding.h"


//Vbyte encoding

uint getEncodedSize(uint in) {
	uint res = 1;
	while (in >= 128) {
		res++;	
		in = in >> 7;		
	}
	return res;
}

uint encodeValue(uint in, byte * out) {
	uint bytesWritten = 0;
	while(1) {
		if (in < 128) {
			out[bytesWritten++] = in;
			break;
		}
		out[bytesWritten++] = 128 | (in&127);
		in = in>>7;
	}
	return bytesWritten;
}

uint decodeValue(byte *vbyte, uint *bytesRead) {
	uint ret = 0;
	uint shift = 0;
	uint temp;
	*bytesRead = 0;
	for (;;shift+=7) {
		temp = vbyte[(*bytesRead)++];
		ret += (temp&0x7F) <<shift;
		if (temp < 128) break;
	}
	return ret;
}

uint locateEntry(byte * _vbyte, uint byteSize, uint nEntry) {
	byte * vbyte = _vbyte;
	while (nEntry) {
		if (*(vbyte++) < 128) nEntry--;
	}
	return vbyte - _vbyte;
}

uint countEntries(byte *vbyte, uint byteSize) {
	byte * end = vbyte + byteSize;
	int counter = byteSize;
	while(vbyte < end) counter -= (*(vbyte++)&0x80)&&1;
	return counter;
}
