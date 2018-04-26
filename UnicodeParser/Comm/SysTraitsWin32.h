/**
*/

#ifndef SYS_TRAITS_WIN32_H_SIM_
#define SYS_TRAITS_WIN32_H_SIM_

#ifndef _WIN32_WINNT            // 指定要求的最低平台是 Windows Vista。
#define _WIN32_WINNT 0x0600 
#endif

#include <WinSock2.h>
#include <windows.h>


#include <string>
#include <vector>
#include <time.h>
#include <assert.h>
#include <stdlib.h>

typedef __int8				int8;
typedef __int16				int16;
typedef __int32				int32;
typedef __int64				int64;

typedef unsigned __int8		uint8;
typedef unsigned __int16	uint16;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;
typedef int64				time_tick;

typedef uint32				tick32;

const int MAX_NAME_SIZE	= 32;


#ifndef ASSERT
#define ASSERT assert
#endif

inline time_tick GetSysTime()
{
	time_t t;
	time(&t);
	return(time_tick)(t);
}
inline time_tick GetSysTick()
{
	return static_cast<time_tick>(GetTickCount64());
}
// inline int64 atoll(const char* str)
// {
// 	return _atoi64(str);
// }
inline tm* LocalTime(tm* m,time_tick tick)
{
	localtime_s(m,&tick);
	return m;
}

inline void FlushSharedMemory(void* data, size_t len)
{
	FlushViewOfFile(data, len);
}

inline void SysFatalError(int error = -1)
{
	exit(error);
}
#endif // 
