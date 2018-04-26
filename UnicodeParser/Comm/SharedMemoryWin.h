#ifndef SharedMemoryWin_h__
#define SharedMemoryWin_h__
#include "SysTraits.h"
#include "sstring.hpp"

static const char* path_ = "./data/";
namespace shared_memory {

struct MemorySink 
{
public:
	enum eState
	{
		MemoryUnknow = -1,
		MemoryOK,
		MemoeryError,
	};
	MemorySink(size_t index,size_t cap,sstring64& name,sstring256& path)
		:index_(index),state_(MemoryUnknow),fullCapacity_(cap),freeCapacity_(0),
		handle_(INVALID_HANDLE_VALUE),fileHandle_(INVALID_HANDLE_VALUE),
		name_(name),path_(path),
		memory_(nullptr),next_(nullptr),isCreate_(false),isUsing_(false)
	{
	}
	size_t  GetIndex()const
	{
		return index_;
	}
	size_t GetState()const
	{
		return state_;
	}
	size_t GetFullCapacity()const
	{
		return fullCapacity_;
	}
	size_t GetFreeCapacity()const
	{
		return freeCapacity_;
	}
	const sstring64& GetName()const
	{
		return name_;
	}
	void* GetMemory()const
	{
		return memory_;
	}
	void* GetMemory(size_t distance)const
	{
		if (distance >= GetFullCapacity())
		{
			return nullptr;
		}
		return ((char*)memory_)+distance;
	}
	ptrdiff_t GetDistanceUncheck(void* mm)
	{
		return (ptrdiff_t)((char*)mm - (char*)memory_);
	}
	bool IsCreate()const
	{
		return isCreate_;
	}
	void SetNext(MemorySink* sink)
	{
		next_ = sink;
	}
	MemorySink* GetNext()const
	{
		return next_;
	}
	bool IsUsing()const
	{
		return isUsing_;
	}
	void SetUsing(bool u)
	{
		isUsing_ = u;
	}
	bool CreateMemory()
	{
		checkFile();
		openFile();
#pragma warning(disable:4267)
		handle_ = CreateFileMapping(fileHandle_,nullptr,PAGE_READWRITE,0,GetFullCapacity(),GetName().c_str());
		if (handle_ == INVALID_HANDLE_VALUE)
		{
			state_ = GetLastError();
			if (state_ == ERROR_ALREADY_EXISTS )
			{
				return OpenMemory();
			}else
			{
				return false;
			}
		}
#pragma warning(default:4267)
		return mapMemory();
	}
	bool OpenMemory()
	{
		handle_ = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,name_.c_str());
		if (handle_ == INVALID_HANDLE_VALUE)
		{
			state_ = GetLastError();
			return false;
		}
		return mapMemory();
	}
	void CloseMemory()
	{
		if (handle_ != INVALID_HANDLE_VALUE)
		{
			if (memory_!=nullptr)
			{
				UnmapViewOfFile(memory_);
			}
			CloseHandle(handle_);
		}
	}
	void ReleaseMemoryFile()
	{
		//CloseMemory();
		sstring256 path = path_;
		path.append(name_.c_str());
		DeleteFile(path.c_str());
	}
	bool IsContain(void* mm,size_t len)
	{
		ptrdiff_t sink_mem = (ptrdiff_t)GetMemory();
		ptrdiff_t pd_mem = (ptrdiff_t)mm;
		if (pd_mem >= sink_mem && (pd_mem+len) <= (sink_mem + fullCapacity_))
		{
			return true;
		}
		return false;
	}
	void FlushMemory()
	{
		FlushViewOfFile(memory_, GetFullCapacity());
	}
	void FlushMemory(void* menm, size_t len)
	{
		FlushViewOfFile(menm, len);
	}
	void FlushMemory(void* mem, size_t begin, size_t len)
	{
		char* mm = ((char*)mem) + begin;
		FlushSharedMemory(mm, len);
	}
private:
	bool mapMemory()
	{
		memory_ = MapViewOfFile(handle_,FILE_MAP_ALL_ACCESS,0,0,0);
		if (!memory_)
		{
			state_ = GetLastError();
			CloseMemory();
			return false;
		}
		state_ = MemoryOK;
		return true;
	}
	void openFile()
	{
		if (fileHandle_ == INVALID_HANDLE_VALUE)
		{
			sstring256 path = path_;
			path.append(name_.c_str());
			fileHandle_ = CreateFile(path.c_str(),
				GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				nullptr,
				OPEN_ALWAYS,FILE_FLAG_RANDOM_ACCESS,
				nullptr);
		}	
	}
	void	checkFile()
	{
#pragma warning(disable:4996)
		FILE* f = nullptr;
		sstring256 path = path_;
		path.append(name_.c_str());
		f = fopen(path.c_str(),"r");
		if (f)
		{
			fclose(f);
			isCreate_ = false;
		}else
		{
			isCreate_ = true;
		}
#pragma warning(default:4996)
	}
	size_t		index_;
	size_t		state_;
	size_t		fullCapacity_;
	size_t		freeCapacity_;
	HANDLE		fileHandle_;
	HANDLE		handle_;
	sstring64	name_;
	sstring256	path_;
	void*		memory_;
	MemorySink*	next_;
	bool		isCreate_;
	bool		isUsing_;
};

}//end of namespace SharedMemory

#endif // SharedMemoryWin_h__

