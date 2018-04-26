//copy from chrome v8 source zone.h
#pragma once
#ifndef ZoneMemory_H__
#define ZoneMemory_H__
#include "SysTraits.h"
#include "MemoryAllocator.h"
class ZoneMemory final
{
public:
	enum {
		kMinimumSegmentSize = 64*KB,
		kMaximumSegmentSize = 1024*MB,
	};
	ZoneMemory();
	~ZoneMemory();
	int Initialize(const char* name, eAllocatorType allocType,size_t init_sgment_size = 20*MB);
	void* New(size_t size);
	template<typename T>
	T* NewArray(size_t length)
	{
		return static_cast<T*>(New(sizeof(T)*length));
	}
	void DeleteAll();
	void Seal(bool is_sealed)
	{
		sealed_ = is_sealed;
	}
	const char* GetName()const
	{
		return name_;
	}
private:
	struct Segment 
	{
		Segment(size_t sz) :next_(nullptr), /*zone_(nullptr),*/ size_(sz) {}
		Segment* GetNext() { return next_; }
		void SetNext(Segment* next) { next_ = next; }
		//ZoneMemory* GetZone() { return zone_; }
		//void SetZone(ZoneMemory* zone) { zone_ = zone; }
		size_t GetSize()const { return size_; }
		size_t GetCapacity()const { return size_ - sizeof(Segment); }
		Segment*	next_;
		//ZoneMemory*	zone_;
		size_t		size_;
	};
	Segment* NewSegment(size_t size);
private:
	Address			position_;
	Address			tail_;
	Segment*		headSegment_;
	Allocator*		allocator_;
	size_t			size_;
	const char*		name_;
	bool			sealed_;
	eAllocatorType	allocatorType_;
};

class ZoneObject {
public:
	// Allocate a new ZoneObject of 'size' bytes in the Zone.
	void* operator new(size_t size, ZoneMemory* zone) { return zone->New(size); }

	// Ideally, the delete operator should be private instead of
	// public, but unfortunately the compiler sometimes synthesizes
	// (unused) destructors for classes derived from ZoneObject, which
	// require the operator to be visible. MSVC requires the delete
	// operator to be public.

	// ZoneObjects should never be deleted individually; use
	// Zone::DeleteAll() to delete all zone objects in one go.
	void operator delete(void*, size_t) {  }
	void operator delete(void* pointer, ZoneMemory* zone) {  }
};

#endif // !ZoneMemory_H__

