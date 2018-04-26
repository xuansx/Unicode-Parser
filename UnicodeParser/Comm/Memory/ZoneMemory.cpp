#include "ZoneMemory.h"


ZoneMemory::ZoneMemory()
	:position_(nullptr),tail_(nullptr),headSegment_(nullptr),allocator_(nullptr),
	size_(0),name_(nullptr),sealed_(false),allocatorType_(kInMemory)
{
}


ZoneMemory::~ZoneMemory()
{
	if (allocator_)
	{
		allocator_->shutdown();
		if (allocatorType_ == kSharedMemory)
		{
			FixedSharedMemoryAllocator::destroy(allocator_);
		}
		else
		{
			MemoryAllocator::destroy(allocator_);
		}
		allocator_ = nullptr;
	}
}
int ZoneMemory::Initialize(const char* name, eAllocatorType allocType,size_t init_sgment_size)
{
	name_ = name;
	if (allocType == kSharedMemory)
	{
		allocator_ = FixedSharedMemoryAllocator::create();
	}
	else
	{
		allocator_ = MemoryAllocator::create();
	}
	allocator_->startup(name, 0);
	if (!allocator_)
	{
		return -1;
	}
	allocatorType_ = allocType;

	NewSegment(init_sgment_size);
	return 0;
}
void* ZoneMemory::New(size_t size)
{
	if (sealed_)
	{
		return nullptr;
	}
	size_t aligned_size = ((size / kMEMORY_ALIGED_SIZE) + 1)*kMEMORY_ALIGED_SIZE;
	if (aligned_size > kMaximumSegmentSize)
	{
		SysFatalError();
		return nullptr;
	}
	else if(aligned_size > tail_ - position_)
	{
		if (aligned_size < kMinimumSegmentSize)
		{
			NewSegment(kMinimumSegmentSize);
		}
		else
		{
			NewSegment(aligned_size);
		}
	}

	Address resPosition = position_;
	position_ += aligned_size;
	return resPosition;
}
void ZoneMemory::DeleteAll()
{
	while (headSegment_)
	{
		Segment* next = headSegment_->GetNext();
		allocator_->free(headSegment_);

		headSegment_ = next;
	}
}
ZoneMemory::Segment* ZoneMemory::NewSegment(size_t size)
{
	size_t segment_size = sizeof(Segment) + size;
	void* address = allocator_->allocate(segment_size);
	Segment* seg = new(address) Segment(size);
	if (!seg)
	{
		SysFatalError();
		return nullptr;
	}

	seg->SetNext(headSegment_);

	Address positionBegin = (Address)address;
	position_ = positionBegin + sizeof(Segment);
	tail_ = position_ + size;
	headSegment_ = seg;

	return seg;
}


