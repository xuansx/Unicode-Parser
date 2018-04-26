#include "MemoryAllocator.h"
#include "sstring.hpp"
#include "SharedMemory.h"
#include "MappingArray.h"
#include "FileIO.h"

#include <stdio.h>
#include <stdlib.h>

#include <fstream>


const static size_t	kMaxMemorySinkCount = _MB(1);
const static size_t	kMaxMemoryCapacity = _GB(32);

struct SinkHistoryArray;
struct MemorySink
{
	static const size_t HEADER_SIZE = sizeof(char) * 64;
	MemorySink(size_t cap)
		:capacity_(cap), isUsing_(false), next_(this), prev_(this),
		startPosition_(0), historyPosition_(0)
	{
	}
	void Init(MemorySink* prevSink)
	{
		if (prevSink)
		{
			startPosition_ = prevSink->startPosition_ + prevSink->capacity_;
		}
		else
		{
			startPosition_ = 0;
		}
	}
	~MemorySink()
	{
		next_ = nullptr;
		capacity_ = 0;
		isUsing_ = false;
	}
	void* Buffer()
	{
		return (void*)&buffer_[0];
	}
	void* Buffer(size_t offset)
	{
		if (offset >= GetCapacity())
		{
			return nullptr;
		}
		char* buf = &buffer_[0];
		return &buf[offset * sizeof(char)];
	}
	void* End()
	{
		return &buffer_[GetCapacity()];
	}
	size_t DistanceFromBegin(void* p)
	{
		char* pBlock = (char*)p;
		return pBlock - &buffer_[0];
	}
	size_t GetCapacity()const
	{
		return capacity_;
	}
	size_t GetStartPosition()const
	{
		return startPosition_;
	}
	void SetHistoryPosition(size_t position)
	{
		historyPosition_ = position;
	}
	size_t GetHistoryPosition()const
	{
		return historyPosition_;
	}
	void* GetHistoryBuffer()
	{
		if (historyPosition_ == 0)
		{
			return nullptr;
		}
		return Buffer(historyPosition_);
	}
	MemorySink* GetNext()const
	{
		return next_;
	}
	bool IsContain(void* p)
	{
		if (p >= Buffer() && p < End())
		{
			return true;
		}
		return false;
	}
	MemorySink* GetPrev()const
	{
		return prev_;
	}
	bool  Greater(MemorySink* other)
	{
		if (!other)
		{
			return true;
		}
		if (capacity_ > other->capacity_)
		{
			return true;
		}
		return false;
	}
	bool operator< (const MemorySink& other)
	{
		if (capacity_ < other.capacity_)
		{
			return true;
		}
		return false;
	}
	void InsertAfter(MemorySink* sink)
	{
		if (sink == this)
		{
			return;
		}
		next_ = sink->next_;
		prev_ = sink;
		sink->next_->prev_ = this;
		sink->next_ = this;
	}
	void Remove()
	{
		if (!next_ || !prev_)
		{
			return;
		}
		if (GetNext() == this)
		{
			return;
		}
		next_->prev_ = prev_;
		prev_->next_ = next_;
		next_ = this;
		prev_ = this;
	}
	void* CheckHistory(size_t& count)
	{
		size_t* buffer = static_cast<size_t*>(GetHistoryBuffer());
		if (!buffer)
		{
			return nullptr;
		}
		count = *buffer++;
		if (count > kMaxMemorySinkCount)
		{
			return nullptr;
		}
		return buffer;
	}
	union
	{
		struct
		{
			MemorySink*	next_;
			MemorySink*	prev_;
			//MemorySink* last_address_;
			//size_t		last_capacity_;
			//MemorySink*	current_address_;
			size_t		capacity_;
			size_t		startPosition_;
			size_t		historyPosition_;
			bool		isUsing_;
		};
		char			reserve[64];
	};
	char				buffer_[64];
};

struct SinkHistory {
	Address			address;
	ptrdiff_t		capacity;
	ptrdiff_t		startPosition;
	void Init(MemorySink* sink)
	{
		address = static_cast<Address>(sink->Buffer());
		capacity = static_cast<ptrdiff_t>(sink->GetCapacity());
		startPosition = static_cast<ptrdiff_t>(sink->GetStartPosition());
	}
	bool Init(SinkHistory* history)
	{
		address = history->address;
		if (reinterpret_cast<ptrdiff_t>(address) >= kMaximumMemorySize)
		{
			return false;
		}
		capacity = history->capacity;
		if (capacity > kMaxMemoryCapacity)
		{
			return false;
		}
		startPosition = history->startPosition;
		if (capacity > kMaxMemoryCapacity)
		{
			return false;
		}
		return true;
	}
	void Init(Address addr, ptrdiff_t cap, ptrdiff_t position)
	{
		address = addr;
		capacity = cap;
		startPosition = position;
	}
	void Init(Address addr, ptrdiff_t cap, SinkHistory* prevSink)
	{
		address = addr;
		capacity = cap;
		if (prevSink)
		{
			startPosition = prevSink->capacity + prevSink->startPosition;
		}
		else
		{
			startPosition = 0;
		}
	}
	inline ptrdiff_t IsIn(Address resolveAddress)
	{
		if (resolveAddress >= address && resolveAddress < (address + capacity))
		{
			return resolveAddress - address + startPosition;
		}
		return -1;
	}
};

struct 	SinkHistoryArray :public MappingArray<SinkHistory>
{
	void Init(MemorySink* sink, size_t count)
	{
		SetLength(count);
		SinkHistory* history = begin();
		while (sink && history != end())
		{
			history->Init(sink);
			++history;
			sink = sink->GetNext();
		}
	}
	bool Load(void* mem)
	{
		SinkHistory* history = static_cast<SinkHistory*>(mem);
		for (size_t i = 0; i < length(); i++)
		{
			SinkHistory* target = at(i);
			if (!target)
			{
				return false;
			}
			target->Init(history++);
		}
		return true;
	}
	inline ptrdiff_t remapping(Address source)
	{
		ptrdiff_t locatePosition = 0;
		for (size_t i = 0; i < length(); i++)
		{
			SinkHistory* sink = at(i);
			if (!sink)
			{
				return -1;
			}
			locatePosition = sink->IsIn(source);
			if (locatePosition > 0)
			{
				return locatePosition;
			}
		}
		return -1;
	}
};

class MemoryAllocator::MemoryAllocatorImpl :public Allocator
{
public:
	MemoryAllocatorImpl()
		:head_(nullptr), tail_(nullptr), freeHead_(nullptr),freeTail_(nullptr), 
		sinkHistoryArray_(nullptr),
		sinkCount_(0), userDataHeadSize_(0)
	{}
	~MemoryAllocatorImpl()
	{
		if (head_)
		{
			destroy(head_);
			head_ = nullptr;
		}
		if (freeHead_)
		{
			destroy(freeHead_);
			freeHead_ = nullptr;
		}
		destorySinkHistoryArray();
	}
	void  startup(const char* name, size_t userDataHeadSize)
	{
		name_ = name;
		userDataHeadSize_ = userDataHeadSize;
	}
	void  shutdown()
	{
		if (head_)
		{
			destroy(head_);
		}
		if (freeHead_)
		{
			destroy(freeHead_);
		}
		head_ = nullptr;
		tail_ = nullptr;
		freeHead_ = nullptr;
	}
	void* address(size_t distance)
	{
		MemorySink* sink = head_;
		do
		{
			if (distance < sink->GetCapacity())
			{
				return sink->Buffer(distance);
			}
			sink = sink->GetNext();
			distance -= sink->GetCapacity();
		} while (sink != head_);
		return nullptr;
	}
	size_t distance(void* address)
	{
		MemorySink* sink = head_;
		size_t		distanceBase = 0;
		do
		{
			if (sink->IsContain(address))
			{
				distanceBase += sink->DistanceFromBegin(address);
				return  distanceBase;
			}
			sink = sink->GetNext();
			distanceBase += sink->GetCapacity();
		} while (sink != head_);
		return END_OF_INDEX;
	}
	void* allocate(size_t len)
	{
		if (len == 0)
		{
			return nullptr;
		}
		MemorySink* sink = _Alloc(len);
		if (sink)
		{
			return sink->Buffer();
		}
		else
		{
			return nullptr;
		}
	}
	void  free(void* block)
	{
		if (!block)
		{
			return;
		}
		ptrdiff_t pointer = (ptrdiff_t)block;
		pointer -= MemorySink::HEADER_SIZE;
		MemorySink* sink = (MemorySink*)pointer;
		if (checkBlockExist(sink))
		{
			recycleSink(sink);
			--sinkCount_;
		}
	}
	void  flush()
	{
		if (!head_)
		{
			return;
		}
		if (name_.empty())
		{
			return;
		}
		sstring256 filename = "./data/";
		MakeDir(filename.c_str());
		filename.append(name_.c_str(), name_.length()).append(".data");
		DelFile(filename.c_str());
		XFile file;
		int* erro = _errno();
		//if (head_ != tail_)
		{
			size_t total_cap = calcTotalCapacityMoreThan2Sink();
			head_->SetHistoryPosition(total_cap);
			createSinkHistoryFromCurrentSinkList();
		}
		if (file.Open(filename.c_str(),XFile::Op_AppendBinaryEx))
		{
			setError(CErr_OK);
			MemorySink* sink = head_;
			while (sink)
			{
				if (!file.Write(sink->Buffer(userDataHeadSize_), sink->GetCapacity() - userDataHeadSize_))
				{
					setError(CErr_WriteFileFailed);
					break;
				}
				sink = sink->GetNext();
				if (sink == head_)
				{
					break;
				}
			}
			if (IsOK()&& 
				sinkHistoryArray_&& 
				sinkHistoryArray_->length() == sinkCount_)
			{
				if(!file.Write(sinkHistoryArray_, sinkHistoryArray_->SizeOfMy()))
				{
					setError(CErr_WriteFileFailed);
				}
			}
			file.Close();
		}
		else
		{
			setError(CErr_OpenFileFailed);
		}
	}
	MemoryRecoverInfo*  recover()
	{
		recoverInfo_.Clear();
		if (name_.empty())
		{
			return &recoverInfo_;
		}
		sstring256 filename = "./data/";
		filename.append(name_.c_str(), name_.length()).append(".data");
		//FILE* hFile = fopen(filename.c_str(), "rb");
		XFile	file;
		if (file.Open(filename.c_str(),XFile::Op_ReadBinaryEx))
		{
			setError(CErr_OK);
			recoverInfo_.capacity = file.Size();
			file.ResetSeek();
			//fseek(hFile, 0, SEEK_SET);
			while (recoverInfo_.capacity > 0)
			{
				MemorySink* sink = _Alloc(recoverInfo_.capacity);
				if (sink)
				{
					char* buf = (char*)sink->Buffer(userDataHeadSize_);
					size_t rlen = 0;
					recoverInfo_.buffer		= sink->Buffer();
					//do {
					//	rlen += fread(&buf[rlen], 1, recoverInfo_.capacity - rlen, hFile);
					//} while (rlen < recoverInfo_.capacity);
					if (!file.Read(buf, recoverInfo_.capacity))
					{
						setError(CErr_ReadFileFailed);
						break;
					}
					//load history array.
					size_t historyCount = 0;
					void*mem = sink->CheckHistory(historyCount);
					createSinkHistoryArray(historyCount);
					if (mem && sinkHistoryArray_)
					{
						if (!sinkHistoryArray_->Load(mem))
						{
							destorySinkHistoryArray();
						}
					}
				}
				break;
			}
			file.Close();
		}
		else
		{
			setError(CErr_OpenFileFailed);
		}
		return & recoverInfo_;
	}
	MemoryRefineInfo*  refine(size_t skipHeaderLen)
	{
		refineInfo_.Clear();
		refineInfo_.skip_head_lenght = skipHeaderLen;
		if (!head_)
		{
			return &refineInfo_;
		}
		if (head_ == tail_)
		{
			refineInfo_.block_count = 1;
			refineInfo_.buffer = head_->Buffer();
			refineInfo_.capacity = head_->GetCapacity();
			return &refineInfo_;
		}
		size_t cap = calcTotalCapacityMoreThan2Sink();
		createSinkHistoryFromCurrentSinkList();
		MemorySink* freshSink = _Alloc(cap);
		MemorySink*	sink = head_;
		MemorySink*	curr = nullptr;
		size_t pointer = 0;
		do
		{
			refineInfo_.block_count += 1;
			memcpy(freshSink->Buffer(pointer), sink->Buffer(skipHeaderLen), sink->GetCapacity() - skipHeaderLen);
			pointer += (sink->GetCapacity() - skipHeaderLen);
			curr = sink;
			sink = sink->GetNext();
			recycleSink(curr);
		} while (sink != head_);
		head_ = tail_ = sink;
		refineInfo_.buffer = head_->Buffer();
		refineInfo_.capacity = head_->GetCapacity();
		return &refineInfo_;
	}
	void  flush(void*, size_t begin, size_t count)
	{
		//nothing...
	}
	void* relocate(void* oldAddress)
	{
		if (!head_)
		{
			return nullptr;
		}
		if (!sinkHistoryArray_)
		{
			return nullptr;
		}
		ptrdiff_t position = sinkHistoryArray_->remapping(reinterpret_cast<Address>(oldAddress));
		if (position < 0)
		{
			return nullptr;
		}
		return head_->Buffer(static_cast<size_t>(position));
	}
private:
	inline MemorySink* fetchFreeSink(size_t cap)
	{
		if (!freeHead_)
		{
			return nullptr;
		}
		MemorySink* sink = freeHead_;
		if (freeHead_ == freeTail_ /*&& freeHead_->GetCapacity() >= cap*/)
		{
			freeHead_ = nullptr;
			freeTail_ = nullptr;
			return sink;
		}
		while (sink->GetCapacity() < cap)
		{
			sink = sink->GetNext();
			if (sink == freeTail_)
			{
				return nullptr;
			}
		}
		if (sink == freeHead_)
		{
			freeHead_ = freeHead_->GetNext();
		}
		if (sink == freeTail_)
		{
			freeTail_ = freeTail_->GetPrev();
		}
		sink->Remove();
		return sink;
	}
	MemorySink* reuseSink(size_t cap)
	{
		MemorySink* sink = fetchFreeSink(cap);
		if (!sink)
		{
			return nullptr;
		}
		if (tail_)
		{
			sink->Init(tail_);
			tail_->InsertAfter(sink);
			tail_ = sink;
		}
		else
		{
			sink->Init(nullptr);
			head_ = tail_ = sink;
		}
		return sink;
	}
	MemorySink* linkNewSink(size_t cap)
	{
		MemorySink* sink = createSink(cap);
		if (tail_)
		{
			sink->InsertAfter(tail_);
			sink->Init(tail_);
			tail_ = sink;
		}
		else 
		{
			sink->Init(nullptr);
			head_ = sink;
			tail_ = sink;
		}
		return sink;
	}
	MemorySink* createSink(size_t cap)
	{
		void* mem = malloc(MemorySink::HEADER_SIZE + cap);
		MemorySink* sink = new(mem)MemorySink(cap);
		return sink;
	}
	MemorySink* _Alloc(size_t cap)
	{
		cap += userDataHeadSize_;
		MemorySink* sink = reuseSink(cap);
		if (!sink)
		{
			sink = linkNewSink(cap);
		}
		if (sink)
		{			
			memset(sink->Buffer(), 0,sink->GetCapacity());
			++sinkCount_;
		}
		return sink;
	}
	void recycleSink(MemorySink* sink)
	{
		sink->Remove();
		if (head_ == sink)
		{
			head_ = nullptr;
			tail_ = nullptr;
		}
		if (!freeHead_)
		{
			freeHead_ = sink;
			freeTail_ = sink;
			return;
		}
		if (freeHead_->Greater(sink))
		{
			freeHead_->InsertAfter(sink);
			freeHead_ = sink;
			return;
		}
		if (freeHead_ == freeTail_)
		{
			sink->InsertAfter(freeTail_);
			freeTail_ = sink;
			return;
		}
		MemorySink* linkSink = freeHead_->GetNext();
		while (linkSink != freeHead_)
		{
			if (linkSink->Greater(sink))
			{
				linkSink = linkSink->GetPrev();
				break;
			}
			else
			{
				linkSink = linkSink->GetNext();
				if (linkSink == freeTail_)
				{
					break;
				}
			}
		}
		sink->InsertAfter(linkSink);
		if (linkSink == freeTail_)
		{
			freeTail_ = sink;
		}
	}
	void destroy(MemorySink* begin)
	{
		MemorySink* sink = begin;
		MemorySink* next = nullptr;
		while (sink)
		{
			next = sink->GetNext();
			::free(sink);
			sink = next;
			if (sink = begin)
			{
				break;
			}
		}
	}
	bool checkBlockExist(MemorySink* sinkObj)
	{
		MemorySink* sink = head_;
		do
		{
			if (sink == sinkObj)
			{
				return true;
			}
			sink = sink->GetNext();
		} while (sink != head_);
		return false;
	}
	size_t calcTotalCapacityMoreThan2Sink()const
	{
		size_t cap = head_->GetCapacity();
		MemorySink* sink = head_->GetNext();
		while (sink != head_)
		{
			cap += sink->GetCapacity();
			sink = sink->GetNext();
		}
		return cap;
	}
	void createSinkHistoryFromCurrentSinkList()
	{
		createSinkHistoryArray(sinkCount_);
		if (!sinkHistoryArray_)
		{
			return;
		}
		sinkHistoryArray_->Init(head_, sinkCount_);
	}
	inline void createSinkHistoryArray(size_t count)
	{
		destorySinkHistoryArray();

		if (count)
		{
			return;
		}
		void* mem = malloc(SinkHistoryArray::SizeOf(count));
		sinkHistoryArray_ = new(mem)SinkHistoryArray();
		//sinkHistory_->Init(head_, count);
	}
	void loadSinkHistoryArray()
	{
		size_t count;
		void* mem = head_->CheckHistory(count);
		if (!mem)
		{
			return;
		}
		createSinkHistoryArray(count);
		if (!sinkHistoryArray_)
		{
			return;
		}
		if (!sinkHistoryArray_->Load(mem))
		{
			free(sinkHistoryArray_);
			sinkHistoryArray_ = nullptr;
			return;
		}
	}
	inline void destorySinkHistoryArray()
	{
		if (sinkHistoryArray_)
		{
			free(sinkHistoryArray_);
			sinkHistoryArray_ = nullptr;
		}
	}
private:
	MemorySink*			head_;
	MemorySink*			tail_;
	MemorySink*			freeHead_;
	MemorySink*			freeTail_;
	size_t				sinkCount_;
	size_t				userDataHeadSize_;
	SinkHistoryArray*	sinkHistoryArray_;
	union {
		MemoryRecoverInfo	recoverInfo_;
		MemoryRefineInfo	refineInfo_;
	};
	sstring32			name_;
};
Allocator* MemoryAllocator::instance()
{
	static MemoryAllocatorImpl instance_;
	return &instance_;
}
Allocator* MemoryAllocator::create()
{
	return new MemoryAllocatorImpl();
}
void MemoryAllocator::destroy(Allocator* ba)
{
	delete ba;
}

class FixedSharedMemoryAllocator::FixedSharedMemoryAllocatorImpl :public Allocator
{
public:
	FixedSharedMemoryAllocatorImpl()
	{

	}
	~FixedSharedMemoryAllocatorImpl()
	{
		sm_.Release();
	}
	void  startup(const char* name, size_t cap)
	{
		sm_.Create(cap, name);
	}
	void  shutdown()
	{
		sm_.Release();
	}
	void* address(size_t distance)
	{
		return sm_.GetAddress((ptrdiff_t)distance);
	}
	size_t distance(void* address)
	{
		return (size_t)sm_.GetDistance(address);
	}
	virtual void* allocate(size_t)
	{
		return sm_.AttachSink();
	}
	virtual void  free(void* block)
	{
		sm_.DetachSink(block);
	}
	virtual void  flush()
	{
		sm_.Flush();
	}
	virtual void  flush(void* mem, size_t begin, size_t count)
	{
		sm_.FlushMemory(mem, begin, count);
	}
	virtual MemoryRecoverInfo*  recover() 
	{ 
		sm_.Recover();
		recoverInfo_.Clear();
		recoverInfo_.buffer = sm_.GetRootSink()->GetMemory();
		recoverInfo_.capacity = sm_.GetTotalCapacity();
		return &recoverInfo_;
	};
	virtual MemoryRefineInfo*   refine(size_t skipHeaderLen) 
	{ 
		refineInfo_.Clear();
		sm_.Refine(skipHeaderLen);
		refineInfo_.buffer = sm_.GetRootSink()->GetMemory();
		refineInfo_.capacity = sm_.GetTotalCapacity();
		return &refineInfo_;
	}
private:
	SharedMemory sm_;
	union {
		MemoryRecoverInfo	recoverInfo_;
		MemoryRefineInfo	refineInfo_;
	};
};

Allocator* FixedSharedMemoryAllocator::create()
{
	return new FixedSharedMemoryAllocator::FixedSharedMemoryAllocatorImpl();
}

void FixedSharedMemoryAllocator::destroy(Allocator* ba)
{
	delete ba;
}
Allocator* FixedSharedMemoryAllocator::instance()
{
	static FixedSharedMemoryAllocator::FixedSharedMemoryAllocatorImpl instance_;
	return &instance_;
}

