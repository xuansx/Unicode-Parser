/*
*/

#ifndef SYS_TRAITS_H_SIM_
#define SYS_TRAITS_H_SIM_

#define SYS_TRAITS_TYPE
#ifdef _WIN32
	#include "SysTraitsWin32.h"
#else
	#include "SysTraitsLinux.h"
#endif

#define SAFE_DELETE(A)	if(A){delete A,A=NULL;}
#define SAFE_DELETE_ARRAY(A)	if(A){delete[] A,A=NULL;}
#define MAKEDW(l,h) (h<<16|l)  
#define LOWW(dw) (dw&0x0000FFFF)
#define HIGHW(dw) (dw>>16)

#define CACHE_LINE_BASE		64

typedef uint8*		Address;

enum:size_t {
	kMEMORY_ALIGED_SIZE = 8,
	kMaximumMemorySize = 0xFFFFFFFFFFFF,
};
const uint32 KB = 1024;
const uint32 MB = KB * KB;
const uint32 GB = KB * KB * KB;


constexpr size_t _MB(size_t n)
{
	return n * 1024 * 1024;
}

constexpr size_t _GB(size_t n)
{
	return n * 1024 * 1024 * 1024;
}
//work thread call only.
time_tick	CheckCurrentWorkTime();

time_tick   GetCurrentWorkTime();

tick32		GetWorkTickCount();

#endif 
