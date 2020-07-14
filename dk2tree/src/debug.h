/*
 * debug.h
 *
 *  Created on: 17/12/2012
 *      Author: guillermo
 *
 *      Utils for debugging code:
 *      - Simple log facade
 *      - Simple ASSERT macro to provide assert with optional messages
 *      - Macros CHECK and CHECKT to print values only when debugging
 *
 *      Macros are based on gcc constructs.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef DEBUG_H_
#define DEBUG_H_

#define L_NEVER 0
#define L_ERROR 1
#define L_WARN 2
#define L_INFO 3
#define L_DEBUG 4
#define L_TRACE 5
#define L_ALL 10

#define LOGLEVEL L_INFO

#ifndef LOGLEVEL
#ifdef NDEBUG
#define LOGLEVEL L_INFO
#else
#define LOGLEVEL L_TRACE
#endif
#endif



#define WHERESTR  "[%s:%d]: "
#define WHEREARG  __FILE__, __LINE__

#define MYPRINT(fmt, where, ...)  do{fprintf(stdout, fmt, where, ##__VA_ARGS__);fflush(stdout);}while(0)
#define MYPRINTERR(fmt, where, ...) do{fprintf(stderr, fmt, where, ##__VA_ARGS__);fflush(stderr);}while(0)

#if LOGLEVEL >= L_TRACE
	#define TRACE(_fmt, ...) MYPRINT("[TRACE]" WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#endif

#if LOGLEVEL >= L_DEBUG
	#define DEBUG(_fmt, ...) MYPRINT("[DEBUG]" WHERESTR _fmt, WHEREARG, ##__VA_ARGS__)
#endif

#if LOGLEVEL >= L_INFO
	#define INFO(_fmt, ...) MYPRINT("[INFO ]" WHERESTR _fmt, WHEREARG, ##__VA_ARGS__)
#endif

#if LOGLEVEL >= L_WARN
	#define WARN(_fmt, ...) MYPRINT("[WARN ]" WHERESTR _fmt, WHEREARG, ##__VA_ARGS__)
#endif

#if LOGLEVEL >= L_ERROR
	#define ERROR(_fmt, ...) MYPRINTERR("[ERROR]" WHERESTR _fmt, WHEREARG, ##__VA_ARGS__)
#endif

#if LOGLEVEL >= L_ALL
	#define TEMP(_fmt, ...) MYPRINT("[TRACE]" WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#endif
#define PRINT(x) INFO("%s=%d\n", #x, x)

#define PRINTIF(cond, ...) do{int __x=(cond);if (__x) {MYPRINT(__VA_ARGS__);}}while(0);

#ifndef NDEBUG
#define ABORT(...) do{ERROR(__VA_ARGS__);abort();} while(0)
#else
#define ABORT(...) do{ERROR(__VA_ARGS__);exit(-1);} while(0)
#endif

#ifndef NDEBUG
	#define ASSERT(expr, ...) do{int __x = (int) expr; if (!__x) {ERROR("Assertion failed: %s\n", #expr); ABORT(__VA_ARGS__);}}while(0)
	#define CHECKT(x,t) DEBUG("%s: "t"\n", #x, x)
	#define CHECK(x) CHECKT(x,"%d")
#endif

#ifndef ERROR
	#define ERROR(...)
#endif
#ifndef WARN
	#define WARN(...)
#endif
#ifndef INFO
	#define INFO(...)
#endif
#ifndef DEBUG
	#define DEBUG(...)
#endif
#ifndef TRACE
	#define TRACE(...)
#endif
#ifndef TEMP
	#define TEMP(...)
#endif
#ifndef ASSERT
	#define ASSERT(...)
#endif
#ifndef CHECK
	#define CHECKT(...)
	#define CHECK(...)
#endif




#endif /* DEBUG_H_ */
