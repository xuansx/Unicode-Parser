
#ifndef SharedMemory_h__
#define SharedMemory_h__

#include "SharedMemoryWin.h"

static const char* PROJECTION_NAME = "genx";
class SharedMemory
{
public:
	enum eSharedMemoryUsage
	{
		USAGE_DATA,
		USAGE_COMM,
		USAGE_LOG,
		USAGE_OTHER,
	};
	SharedMemory(size_t index = 0)
		:prefix_name_("local_"), projection_name_(PROJECTION_NAME), 
		index_(index), sink_(nullptr), sink_cap_(4096),
		total_capacity_(0)
	{
	}
	~SharedMemory()
	{
		Release();
	}
	bool Create(size_t sink_cap,const char* name,eSharedMemoryUsage u = USAGE_DATA);
	shared_memory::MemorySink* CreateSink();
	void Release(bool delFile = false);
	void Flush(void* mem, size_t len);
	void FlushMemory(void* mem, size_t begin, size_t len);
	void Flush();
	void Recover();
	void Refine(size_t skipHeadLen = 0);
	bool  IsCreate()const
	{
		return sink_->IsCreate();
	}
	void* Memory()
	{
		return sink_->GetMemory();
	}
	shared_memory::MemorySink* GetRootSink()const
	{
		return sink_;
	}
	bool IsAvailable()const
	{
		if (sink_)
		{
			return true;
		}
		return false;
	}
	void SetIndex(size_t index)
	{
		index_ = index;
	}
	void SetPrefix(const char* prefix)
	{
		prefix_name_.clear();
		prefix_name_ = prefix;
	}
	void SetProjection(const char* proj)
	{
		projection_name_.clear();
		projection_name_ = proj;
	}
	size_t		GetSinkCapacity()const
	{
		return sink_cap_;
	}
	size_t		GetTotalCapacity()const
	{
		return total_capacity_;
	}
	void*		AttachSink();
	bool		DetachSink(void* mem);
	void*		GetAddress(ptrdiff_t index);
	ptrdiff_t		GetDistance(void* mem);
private:
	shared_memory::MemorySink* GetContainer(void* mem,size_t len);
	shared_memory::MemorySink* OpenFileSink(size_t index);
private:
	shared_memory::MemorySink* sink_;
	size_t		sink_cap_;
	size_t		total_capacity_;
	size_t		index_;
	sstring8	ext_name_;
	sstring64	name_;
	sstring256	path_;
	sstring32	prefix_name_;
	sstring32	projection_name_;
};

#endif // SharedMemory_h__

