#include "Log.h"

LogSink::LogSink(int32 sinkid,int32 svrID,const char* profix_name ,const char* proj_name)
	:sinkID(sinkid),serverID(svrID),memory(serverID)
{
	if (profix_name)
	{
		memory.SetPrefix(profix_name);
	}
	if (proj_name)
	{
		memory.SetProjection(proj_name);
	}
	recreate();
}

LogSink::~LogSink()
{
	if (memory.IsAvailable())
	{
		memory.Release();
	}
}


void LogSink::recreate()
{
	if (memory.IsAvailable())
	{
		memory.Release();
	}
	updateTimestamp(0);
	memory.Create(LOG_FILE_MAX_LEN,timestampBuf.c_str(),SharedMemory::USAGE_LOG);
	logstr.reset((char*)memory.Memory(),LOG_FILE_MAX_LEN);
}

void LogSink::updateTimestamp(time_tick tick,bool bFile)
{
	if (tick == 0)
	{
		tick = GetCurrentWorkTime();
	}
	lastTick = tick;
	int len = 0;
#if _MSC_VER
	tm time_m;
	localtime_s(&time_m,&tick);
	tm* t = &time_m;
#else
	time_t	tk = (time_t)tick;
	tm* t = localtime(&tk);
#endif
	timestampBuf.reset();
	if (!bFile)
	{
		timestampBuf.printf("[0][%02d%02d%02d%02d%02d]",t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	}else{
		timestampBuf.printf("%02d%02d%02d%02d%02d%02d",(t->tm_year-100),t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	}
}
void LogManager::Startup(int32 svrID)
{
	serverID = svrID;
	CreateSink(0);
}
void LogManager::Shutdown()
{
	for (uint8 i = 0; i < sinks.capacity(); ++i)
	{
		if (sinks[i])
		{
			delete sinks[i];
		}
	}
	sinks.reset();
}
LogSink* LogManager::CreateSink(uint8 sinkID,const char* prefix_name ,const char* proj_name )
{
	LogSink* sink = new LogSink(sinkID,serverID,prefix_name,proj_name);
	sinks[sinkID] = sink;
	return sink;
}

