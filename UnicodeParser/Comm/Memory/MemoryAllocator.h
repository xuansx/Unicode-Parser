#pragma once
#ifndef MemoryAllocator_H_
#define MemoryAllocator_H_
#include "MemoryComm.h"

#include <limits>

struct Allocator:public AllocatorBase
{
	Allocator()
	{
	}
	virtual ~Allocator()
	{
	}
	virtual void  startup(const char* name,size_t userDataHeadSize) = 0;
	virtual void  shutdown() = 0;
	/*
	根据地址偏移量给定实际地址
	*/
	virtual void* address(size_t distance) = 0;
	/*
	根据给定的地址返回相对内存块起始地址的偏移量
	*/
	virtual size_t distance(void* address) = 0;
	virtual void* allocate(size_t len) = 0;
	virtual void  free(void* block) = 0;
	inline  void deallocate(void* block)
	{
		free(block);
	}
	virtual MemoryRecoverInfo*  recover() { return nullptr; };
	virtual MemoryRefineInfo*   refine(size_t skipHeaderLen) { return nullptr; }
	//重新定位地址，对旧的内存地址，转换到新的有效地址
	virtual void* relocate(void* oldAddress) { return oldAddress; }
	virtual void  flush() = 0;
	virtual void  flush(void*, size_t begin, size_t count) = 0;
};

struct SimpleAllocator :public Allocator
{
	virtual void  startup(const char* name, size_t userDataHeadSize)
	{
	}
	virtual void  shutdown()
	{
	}
	/*
	not support
	*/
	virtual void* address(size_t distance)
	{
		return nullptr;
	}
	/*
	not support
	*/
	virtual size_t distance(void* address)
	{
		return 0;
	}
	virtual void* allocate(size_t len)
	{
		return malloc(len);
	}
	virtual void  free(void* block)
	{
		return ::free(block);
	}
	inline  void deallocate(void* block)
	{
		free(block);
	}
	virtual void  flush() {};
	virtual void  flush(void*, size_t begin, size_t count) {};
};

class SimpleAllocatorInstance
{
	static Allocator* instance()
	{
		static SimpleAllocator alloc_s;
		return &alloc_s;
	}
};

template <typename T>
class CareStdAllocator {
public:
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	template <class O>
	struct rebind {
		typedef CareStdAllocator<O> other;
	};

	//#ifdef V8_CC_MSVC
	// MSVS unfortunately requires the default constructor to be defined.
	CareStdAllocator() : CareStdAllocator(nullptr) { UNREACHABLE(); }
	//#endif
	explicit CareStdAllocator(Allocator* allocator) throw() : allocator_(allocator) {}
	explicit CareStdAllocator(const CareStdAllocator& other) throw()
		: CareStdAllocator<T>(other.allocator_) {}
	template <typename U>
	CareStdAllocator(const CareStdAllocator<U>& other) throw()
		: CareStdAllocator<T>(other.allocator_) {}
	template <typename U>
	friend class CareStdAllocator;

	T* address(T& x) const { return &x; }
	const T* address(const T& x) const { return &x; }

	T* allocate(size_t n, const void* hint = 0) {
		return static_cast<T*>(allocator_->allocate(len));
	}
	void deallocate(T* p, size_t) {
		//allocator_->free(p);
	}
	T* relocate(T*p)
	{
		return static_cast<T*>(allocator_->relocate(p));
	}
	size_t max_size() const throw() {
		return UINT_MAX / sizeof(T);
	}
	template <typename U, typename... Args>
	void construct(U* p, Args&&... args) {
		void* v_p = const_cast<void*>(static_cast<const void*>(p));
		new (v_p) U(std::forward<Args>(args)...);
	}
	template <typename U>
	void destroy(U* p) {
		p->~U();
	}

	bool operator==(CareStdAllocator const& other) const {
		return allocator_ == other.allocator_;
	}
	bool operator!=(CareStdAllocator const& other) const {
		return allocator_ != other.allocator_;
	}

	Allocator* allocator() { return allocator_; }

private:
	Allocator* allocator_;
};

enum eAllocatorType {
	kInMemory,
	kSharedMemory,
};
//Allocator关闭时自动整合所有碎片内存refine
class MemoryAllocator
{
	class MemoryAllocatorImpl;
public:
	static Allocator* instance();
	static Allocator* create();
	static void destroy(Allocator* ba);
};

class FixedSharedMemoryAllocator
{
public:
	class FixedSharedMemoryAllocatorImpl;
	static Allocator* create();
	static void destroy(Allocator* ba);
	static Allocator* instance();
};


#endif // !MemoryAllocator_H_
