#ifndef __TIMING_H__
#define __TIMING_H__


#include <sys/types.h>
#include <sys/resource.h>
#include <sys/times.h>

double ticks;
struct tms t1,t2,t3;

void startTimer() {
	times (&t1);
}

double timeFromLast() {
	times (&t3);
	return ((t3.tms_utime+t3.tms_stime)-(t2.tms_utime+t2.tms_stime))/ticks;
	t2 = t3;
}
double timeFromBegin() {
	times (&t2);
	return ((t2.tms_utime+t2.tms_stime)-(t1.tms_utime+t1.tms_stime))/ticks;
//	return ((t2.tms_utime)-(t1.tms_utime))/ticks;
}

#endif
