
#include "SysTraits.h"

volatile static time_tick sBeginWorkTime = GetSysTime();
volatile static time_tick sCurrentWorkTime = GetSysTime();
volatile static tick32	  sWorkTickCount = 0;

time_tick	CheckCurrentWorkTime()
{
	sCurrentWorkTime	= GetSysTime();
	sWorkTickCount		= static_cast<tick32>(sCurrentWorkTime - sBeginWorkTime);
	return sCurrentWorkTime;
}

time_tick   GetCurrentWorkTime()
{
	return sCurrentWorkTime;
}

tick32		GetWorkTickCount()
{
	return sWorkTickCount;
}

