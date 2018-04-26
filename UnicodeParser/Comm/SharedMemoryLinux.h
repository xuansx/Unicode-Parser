
#ifndef SharedMemoryLinux_h__
#define SharedMemoryLinux_h__
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "SysTraits.h"
#include "sstring.hpp"
namespace shared_memory {
struct MemorySink
{
	enum eState
	{
		MemoryUnknow = -1,
		MemoryOK,
		MemoeryError,
	};
	MemorySink(uint32 index ,uint32 cap,sstring64& name)
		:index_(index),state_(MemoryUnknow),fullCapacity_(cap),freeCapacity_(0),
		shmid(0),key_(0),name_(name),memory_(NULL),next_(NULL),isCreate_(false)
	{
	}
	uint32  GetIndex()const
	{
		return index_;
	}
	uint32 GetState()const
	{
		return state_;
	}
	uint32 GetFullCapacity()const
	{
		return fullCapacity_;
	}
	uint32 GetFreeCapacity()const
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
	bool CreateMemory()
	{
		createKey();
		shmid = shmget(key_,fullCapacity_,IPC_CREAT|0666);
		if (shmid < 0 )
		{
			state_ = error();
			return false;
		}
		state_ = MemoryOK;
		return mapMemory();
	}
	bool OpenMemory()
	{
		return CreateMemory();
	}
	void CloseMemory()
	{
		checkFile();
		if (shmdt(memory_) < 0)
		{
			state_ = error();
		}
		shmid_ds ds;
		if(shmctl(shmid,IPC_STAT,&ds) == 0)
		{
			if (ds.shm_nattch == 0)
			{
				if(shmctl(shmid,IPC_RMID,NULL)<0)
				{
					state_ = error();
				}
			}
			return ;
		}
		memory_ = 0;
		shmid   = 0;
	}
	bool mapMemory()
	{
		memory_ = shmat(shmid,NULL,0);
		if (memory_ ==(void*)-1)
		{
			state_ = error();
			memory_ = NULL;
			return false;
		}
		return true;
	}
	void createKey()
	{
		key_ = ftok(name_.c_str(),index_);
	}
	int error()
	{
		return errno;
	}

	void	checkFile()
	{
		FILE* f = nullptr;
		f = fopen(name_.c_str(),"r");
		if (f)
		{
			fclose(f);
			isCreate_ = false;
		}else
		{
			isCreate_ = true;
		}
	}
	uint32		index_;
	uint32		state_;
	uint32		fullCapacity_;
	uint32		freeCapacity_;
	int			shmid;
	key_t		key_;
	sstring64	name_;
	void*		memory_;
	MemorySink*	next_;
	bool		isCreate_;
};

}//end of namespace SharedMemory
#endif // SharedMemoryLinux_h__
