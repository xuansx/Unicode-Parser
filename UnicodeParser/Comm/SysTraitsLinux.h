/**
*/

#ifndef SYS_TRAITS_LINUX_H_SIM_
#define SYS_TRAITS_LINUX_H_SIM_

#include <stdint.h>
#include <time.h>
#include <string>
#include <memory.h>
#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>

#ifndef FALSE
#define FALSE (0)
#endif

typedef int8_t				int8;
typedef int16_t				int16;
typedef int32_t				int32;
typedef int64_t				int64;

typedef uint8_t				uint8;
typedef uint16_t			uint16;
typedef uint32_t			uint32;
typedef uint64_t			uint64;
typedef time_t				time_tick;
typedef uint32				tick32;
#ifdef _DEBUG
#ifndef ASSERT
#define ASSERT assert
#endif
#else
#ifndef ASSERT
#define ASSERT  
#endif
#endif
const int MAX_NAME_SIZE	= 32;

inline time_tick GetSysTime()
{
	time_t t;
	time(&t);
	return static_cast<time_tick>(t);
}

inline time_tick GetSysTick()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	time_tick  tt = tv.tv_sec*1000 + tv.tv_usec/1000;
	return tt;
}
inline tm* LocalTime(tm* m,time_tick tick)
{
	tm* time_struct = localtime(&tick);
	memcpy(m,time_struct,sizeof(tm));
	return m;
}

void SysFatalError(int error = -1)
{
	exit(error);
}

#endif //
