#include "SharedMemory.h"

bool SharedMemory::Create(size_t cap,const char* name,eSharedMemoryUsage u)
{
	if (cap != 0)
	{
		sink_cap_ = cap;
	}
	name_.clear();
	name_.append(prefix_name_.c_str(), prefix_name_.length()).append('_').append(projection_name_.c_str(), projection_name_.length()).append("_").append(name);// append('_').append(index_);
	path_.reset();
	path_.append("./",2);
	ext_name_.reset();
	if (u == USAGE_DATA)
	{
		ext_name_.append(".data");		
		path_.append("data/");
	}else if(u == USAGE_COMM)
	{
		ext_name_.append(".comm");
		path_.append("data/");
	}else if (u == USAGE_LOG)
	{
		ext_name_.append(".log");
		path_.append("log/");
	}else
	{
		ext_name_.append(".bin");
		path_.append("bin/");
	}
	Recover();
	if (!sink_)
	{
		return CreateSink() != nullptr;
	}
	return true;
}
shared_memory::MemorySink* SharedMemory::CreateSink()
{
	sstring64 name = name_;
	name_.append('_').append((int)index_).append(ext_name_.begin(),ext_name_.length());
	shared_memory::MemorySink* sink = new shared_memory::MemorySink(index_,sink_cap_,name_, path_);
	if (!sink->CreateMemory())
	{
		delete sink;
		return nullptr;
	}

	index_ += 1;
	if (!sink_)
	{
		sink_ = sink;
	}
	else
	{
		shared_memory::MemorySink* next = sink_;
		while (next->GetNext())
		{
			next = next->GetNext();
		}
		next->SetNext(sink);
	}
	total_capacity_ += sink->GetFullCapacity();
	return sink;
}
shared_memory::MemorySink* SharedMemory::OpenFileSink(size_t index)
{
	sstring64 name = name_;
	name_.append('_').append((int)index).append(ext_name_.begin(), ext_name_.length());
	shared_memory::MemorySink* sink = new shared_memory::MemorySink(index, sink_cap_, name_, path_);
	
	if (!sink->OpenMemory())
	{
		delete sink;
		return nullptr;
	}

	return sink;
}
void SharedMemory::Release(bool delFile)
{
	shared_memory::MemorySink* head;
	while (sink_)
	{
		head = sink_->GetNext();
		sink_->CloseMemory();
		if (delFile)
		{
			sink_->ReleaseMemoryFile();
		}
		SAFE_DELETE(sink_);
		sink_ = head;
	}
	sink_ = nullptr;
	total_capacity_ = 0;
}

shared_memory::MemorySink* SharedMemory::GetContainer(void* mem, size_t len)
{
	if (sink_->IsContain(mem,len))
	{
		return sink_;
	}
	shared_memory::MemorySink* next = sink_->GetNext();
	while (next)
	{
		if (next->IsContain(mem,len))
		{
			return next;
		}
		next = next->GetNext();
	}
	return nullptr;
}

void*		SharedMemory::GetAddress(ptrdiff_t index)
{
	if (index < (ptrdiff_t)sink_->GetFullCapacity())
	{
		return sink_->GetMemory(index);
	}
	index -= sink_->GetFullCapacity();
	shared_memory::MemorySink* next = sink_->GetNext();
	while (next)
	{
		if (index < (ptrdiff_t)next->GetFullCapacity())
		{
			return next->GetMemory(index);
		}
		index -= next->GetFullCapacity();
		next = next->GetNext();
	}
	return nullptr;
}
ptrdiff_t		SharedMemory::GetDistance(void* mem)
{
	if (sink_->IsContain(mem, 1))
	{
		return sink_->GetDistanceUncheck(mem);
	}
	shared_memory::MemorySink* next = sink_->GetNext();
	while (next)
	{
		if (next->IsContain(mem, 1))
		{
			return next->GetDistanceUncheck(mem);
		}
		next = next->GetNext();
	}
	return size_t(-1);
}
void SharedMemory::FlushMemory(void* mem, size_t begin, size_t len)
{
	shared_memory::MemorySink* sink = GetContainer(mem, len);
	if (sink)
	{
		sink_->FlushMemory(mem, begin, len);
	}
}
void SharedMemory::Flush(void* mem, size_t len)
{
	shared_memory::MemorySink* sink = GetContainer(mem, len);
	if (sink)
	{
		sink_->FlushMemory(mem, len);
	}
}
void SharedMemory::Flush()
{
	shared_memory::MemorySink* sink = sink_;
	while (sink)
	{
		sink->FlushMemory();
		sink = sink->GetNext();
	}
}
void SharedMemory::Recover()
{
	if (sink_)
	{
		return;
	}
	do
	{
		shared_memory::MemorySink* sink = OpenFileSink(index_);
		if (sink == nullptr)
		{
			break;
		}
		index_ += 1;
		if (!sink_)
		{
			sink_ = sink;
		}
		else
		{
			shared_memory::MemorySink* next = sink_;
			while (next->GetNext())
			{
				next = next->GetNext();
			}
			next->SetNext(sink);
		}
		total_capacity_ += sink->GetFullCapacity();
	} while (true);
}

void SharedMemory::Refine(size_t skipHeadLen)
{
	if (index_ < 2)
	{
		return;
	}
	//
	char* temp_buffer = new char[GetTotalCapacity() - skipHeadLen*index_];
	char* ptr = temp_buffer;
	memset(temp_buffer, 0, GetTotalCapacity());
	shared_memory::MemorySink* next = sink_;
	while (next)
	{
		if (next != sink_)
		{
			memcpy(ptr, next->GetMemory(), next->GetFullCapacity());
		}
		else
		{
			memcpy(ptr, next->GetMemory(skipHeadLen), next->GetFullCapacity()-skipHeadLen);
		}
		memcpy(ptr, next->GetMemory(), next->GetFullCapacity());
		ptr += next->GetFullCapacity();
		next = next->GetNext();
	}

	sstring64 name = name_;
	name_.append('_').append((int)0).append(ext_name_.begin(), ext_name_.length());
	shared_memory::MemorySink* sink = new shared_memory::MemorySink(0, GetTotalCapacity(), name_, path_);
	if (!sink->CreateMemory())
	{
		delete[] temp_buffer;
		delete sink;
		return;
	}
	else
	{
		Release();
		memcpy(sink->GetMemory(), temp_buffer, GetTotalCapacity() - skipHeadLen*index_);
		delete[] temp_buffer;
	}
	sink_ = sink;
	index_ = 1;
	sink_cap_ = sink->GetFullCapacity();
}
void* SharedMemory::AttachSink()
{
	shared_memory::MemorySink* sink = sink_;
	while (sink)
	{
		if (!sink->IsUsing())
		{
			sink->SetUsing(true);
			return sink->GetMemory();
		}
		sink = sink->GetNext();
	}
	sink = CreateSink();
	if (sink)
	{
		sink->SetUsing(true);
		return sink->GetMemory();
	}
	return nullptr;
}
bool		SharedMemory::DetachSink(void* mem)
{
	shared_memory::MemorySink* sink = sink_;
	while (sink)
	{
		if (sink->IsContain(mem,sink_cap_))
		{
			sink->SetUsing(false);
			return true;
		}
		sink = sink->GetNext();
	}
	return false;
}

