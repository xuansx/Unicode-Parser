#pragma once
#ifndef MemoryComm_H_
#define MemoryComm_H_

#include "SysTraits.h"
#include "CaseError.h"

#include <stdlib.h>
#include <exception>



struct stdmem
{
	static void* alloc(size_t size)
	{
		return malloc(size);
	}
	static void destory(void* b)
	{
		free(b);
	}
};

class AllocatorBase
{
public:
	static const size_t END_OF_INDEX = size_t(-1);
	static const ptrdiff_t	UNREF_DIFF = -1;
	static const size_t MIN_BLOCK_CAP = 2048;
	eCaseError GetLastError()const
	{
		return lastError_;
	}
	bool IsOK()const
	{
		return lastError_ == CErr_OK;
	}
protected:
	void setError(eCaseError err)
	{
		lastError_ = err;
	}
private:
	eCaseError	lastError_ = CErr_OK;
};

class SinkMemoryException :public std::exception
{
public:
	SinkMemoryException(const char* msg)
		:exception(msg)
	{}
};

struct MemoryRefineInfo
{
	void*  buffer;
	size_t capacity;
	size_t skip_head_lenght;
	size_t block_count;
	void Clear()
	{
		skip_head_lenght = 0;
		block_count = 0;
		capacity = 0;
		buffer = nullptr;
	}
};
struct MemoryRecoverInfo
{
	void*	buffer;
	size_t	capacity;
	bool IsAvailable()
	{
		if (buffer && capacity >= AllocatorBase::MIN_BLOCK_CAP)
		{
			return true;
		}
		return false;
	}
	void Clear()
	{
		buffer = nullptr;
		capacity = 0;
	}
};


#endif // !MemoryComm_H_
