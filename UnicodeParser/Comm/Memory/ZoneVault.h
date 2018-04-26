//vault is a fixed array block linked list;expand by a fixed array block size.
//each element marked by bitset bit distinguishing.
#pragma once
#ifndef ZoneVault_H_
#define ZoneVault_H_

#include "SysTraits.h"
#include "ZoneMemory.h"

#include <bitset>

template<typename T, int N>
class ZoneVault
{
public:
	template<typename BT>
	class Vault_Iterator
	{
	public:
		typedef typename BT							BlockType;
		typedef typename BlockType::IndexType		IndexType;
		typedef typename BlockType::PointerType		PointerType;
		typedef typename BlockType::ReferenceType	ReferenceType;
		typedef Vault_Iterator<BT>					ThisType;
		Vault_Iterator()
			:block_(nullptr), index_(0)
		{}
		Vault_Iterator(BlockType* root)
			:block_(root), index_(0)
		{
			if (!block_->GetFirstElement(index_))
			{
				index_ = 0;
				BlockType* tmp = nullptr;
				while (block_->GetNext())
				{
					block_ = getNextBlockPtr(block_);
					if (!block_)
					{
						break;
					}
					if (block_->GetFirstElement(index_))
					{
						tmp = block_;
						break;
					}
				}
				if (!tmp)
				{
					block_ = nullptr;
					index_ = 0;
				}
			}
		}
		ThisType& operator++()
		{
			if (!block_)
			{
				return *this;
			}
			++index_;
			if (!block_->GetFirstElement(index_))
			{
				index_ = 0;
				while (block_->GetNext())
				{
					block_ = getNextBlockPtr(block_)
						if (!block_)
						{
							break;
						}
					if (block_->GetFirstElement(index_))
					{
						goto LPLUSPLUS_LEFT_END;
					}
				}
				block_ = nullptr;
				index_ = 0;
			}
		LPLUSPLUS_LEFT_END:
			return *this;
		}
		inline ThisType operator++(int)
		{
			if (!block_)
			{
				return *this;
			}
			ThisType temp(block_, index_);

			++index_;
			if (!block_->GetFirstElement(index_))
			{
				index_ = 0;
				while (block_->GetNext())
				{
					block_ = getNextBlockPtr(block_);
					if (!block_)
					{
						break;
					}
					if (block_->GetFirstElement(index_))
					{
						goto LPLUSPLUS_RIGHT_END;
					}
				}
				block_ = nullptr;
				index_ = 0;
			}
		LPLUSPLUS_RIGHT_END:
			return temp;
		}
		PointerType operator->()
		{
			return block_->Get(index_);
		}
		ReferenceType operator*()const
		{
			return *(block_->Get(index_));
		}
		bool operator==(ThisType& other)const
		{
			return (block_ == other.block_) && (index_ == other.index_);
		}
		bool operator!=(ThisType& other)const
		{
			return (block_ != other.block_) || (index_ != other.index_);
		}
		operator bool()
		{
			return block_ != nullptr;
		}
		IndexType GetIndex()const
		{
			return index_;
		}
		PointerType Get()
		{
			return block_->Get(index_);
		}
	private:
		inline BlockType* getNextBlockPtr(BlockType* prev)
		{
			return prev->GetNext();
		}
	private:
		IndexType		index_;
		BlockType*		block_;
	};

	template<typename T, int N>
	struct BlockT
	{
		typedef size_t	IndexType;
		typedef T							ElementType;
		typedef	uint32_t					SizeType;
		typedef ElementType*				PointerType;
		typedef ElementType&				ReferenceType;
		typedef BlockT<ElementType, N>		BlockType;
		static const size_t ELEMENT_SIZE = sizeof(ElementType);
		static const size_t CAPACITY_SIZE = ELEMENT_SIZE*N;
	public:
		static SizeType Capacity()
		{
			return N;
		}
		BlockT()
			:next_(nullptr), count_(0)
		{
		}
		//获得一个空格子以用来存储数据{}
		ElementType* Attach(size_t& totalCnt)
		{
			ASSERT(count_ <= N);
			if (!mask_.test(count_))
			{
				ElementType* element = touch(count_);
				mask_.set(count_);
				++count_;
				++totalCnt;
				return element;
			}
			for (size_t i = 0; i < N; ++i)
			{
				if (!mask_.test(i))
				{
					mask_.set(i);
					++count_;
					++totalCnt;
					return touch(i);
				}
			}
			ASSERT(0);
			return nullptr;
		}
		ElementType* Attach(size_t index, size_t& totalCnt)
		{
			ASSERT(count_ <= N);
			if (!mask_.test(index))
			{
				mask_.set(index);
				++count_;
				++totalCnt;
				return touch(index);
			}
			ASSERT(0);
			return nullptr;
		}
		ElementType* AttachOutIndex(size_t& index, size_t& totalCnt)
		{
			ASSERT(count_ <= N);
			for (size_t i = 0; i < N; ++i)
			{
				if (!mask_.test(i))
				{
					mask_.set(i);
					++count_;
					++totalCnt;
					index = i;
					return touch(index);
				}
			}
			ASSERT(0);
			return nullptr;
		}
		bool GetFirstElement(size_t& index)
		{
			if (index >= N)
			{
				return false;
			}
			for (size_t i = index; i < N; ++i)
			{
				if (mask_.test(i))
				{
					index = i;
					return true;
				}
			}
			return false;
		}
		bool Detach(size_t index, size_t& totalCnt)
		{
			ASSERT(count_ > 0);
			if (mask_.test(index))
			{
				mask_.set(index, false);
				--count_;
				--totalCnt;
				return true;
			}
			return false;
		}
		bool Detach(ElementType* vt, size_t& totalCnt)
		{
			if (vt >= Begin() || vt < End())
			{
				ASSERT(count_ > 0);
				IndexType index = static_cast<IndexType>(vt - Begin());
				ASSERT(index < N);
				if (mask_.test(index))
				{
					mask_.set(index, false);
					--count_;
					--totalCnt;
					return true;
				}
				else
				{
					ASSERT(0);
				}
			}
			return false;
		}
		bool Detach(ElementType* vt)
		{
			if (vt >= Begin() || vt < End())
			{
				ASSERT(count_ > 0);
				size_t index = static_cast<size_t>(vt - Begin());
				ASSERT(index < N);
				if (mask_.test(index))
				{
					mask_.set(index, false);
					--count_;
					return true;
				}
				else
				{
					ASSERT(0);
				}
			}
			return false;
		}
		ElementType* Get(size_t index)
		{
			if (mask_.test(index))
			{
				return touch(index);
			}
			return nullptr;
		}
		ElementType* operator [](size_t index)
		{
			return Get(index);
		}
		bool GetIndex(ElementType* element, size_t& index)
		{
			if (IsContain(element))
			{
				index = 0;
				index += static_cast<size_t>(element - Begin());
				return true;
			}
			return false;
		}
		//protected:
		SizeType GetCount()const
		{
			return count_;
		}
		bool IsFull()const
		{
			return count_ == N;
		}
		bool IsEmpty()const
		{
			return count_ == 0;
		}
		ElementType* Begin()
		{
			return touch(0);
		}
		ElementType* End()
		{
			return touch(N);
		}
		bool IsContain(ElementType* et)
		{
			if (et >= Begin() && et < End())return true;
			return false;
		}
		void Clean()
		{
			memset(elements_, 0, sizeof(elements_));
			count_ = 0;
			mask_.reset();
		}
		void SetNext(BlockType* block)
		{
			next_ = block;
		}
		BlockType* GetNext()const
		{
			return next_;
		}
	private:
		ElementType* touch(IndexType index)
		{
			char* elem = &elements_[ELEMENT_SIZE * index];
			return reinterpret_cast<ElementType*>(elem);
		}
	private:
		SizeType		count_;
		BlockType*		next_;
		std::bitset<N>	mask_;
		char			elements_[CAPACITY_SIZE];
	};
public:
	typedef T									ElementType;
	typedef size_t								IndexType;
	typedef BlockT<T, N>						BlockType;
	typedef ZoneVault<T, N>						ThisType;
	typedef Vault_Iterator<BlockType>			Iterator;
public:
	ZoneVault(ZoneMemory* zone)
		:elementCount_(0), blockCount_(1), head_(nullptr),zone_(zone)
	{
		BlockType* root = new(zone_->New(sizeof(BlockType)))BlockType();

		head_ = (root);
	}
	~ZoneVault()
	{
		freeBlock();
	}
	bool RestoreInitialize(ptrdiff_t ptrdiff);
	ElementType* Attach()
	{
		if (!GetRoot()->IsFull())
		{
			return GetRoot()->Attach(elementCount_);
		}
		if (!IsFull())
		{
			BlockType* bt = getNextBlockPtr(GetRoot());
			ASSERT(bt != nullptr);
			while (bt)
			{
				if (!bt->IsFull())
				{
					return bt->Attach(elementCount_);
				}
				else
				{
					if (bt->GetNext())
					{
						bt = getNextBlockPtr(bt);
					}
					else
					{
						ASSERT(0);
					}
				}
			}
		}
		else
		{
			BlockType* bt = newBlock();
			if (bt)
			{
				return bt->Attach(elementCount_);
			}
		}
		return nullptr;
	}
	ElementType* Attach(IndexType index)
	{
		ASSERT(index >= 0);
		BlockType* bt = GetRoot();

		while (index >= N)
		{
			index -= N;
			if (bt->GetNext())
			{
				bt = getNextBlockPtr(bt);
			}
			else
			{
				BlockType* next = newBlock();
				bt = next;
			}
		}
		return bt->Attach(index, elementCount_);
	}
	ElementType* AttachOutIndex(IndexType& index)
	{
		if (!GetRoot()->IsFull())
		{
			return GetRoot()->AttachOutIndex(index, elementCount_);
		}
		if (!IsFull())
		{
			BlockType* bt = getNextBlockPtr(GetRoot());
			ASSERT(bt != nullptr);
			while (bt)
			{
				if (!bt->IsFull())
				{
					return bt->AttachOutIndex(index, elementCount_);
				}
				else
				{
					if (bt->GetNext())
					{
						bt = getNextBlockPtr(bt);
					}
					else
					{
						ASSERT(0);
					}
				}
			}
		}
		else
		{
			BlockType* bt = newBlock();
			if (bt)
			{
				return bt->AttachOutIndex(index, elementCount_);
			}
		}
		return nullptr;
	}
	bool Detach(ElementType* et)
	{
		ASSERT(elementCount_ > 0);
		if (GetRoot()->IsContain(et))
		{
			return GetRoot()->Detach(et, elementCount_);
		}
		BlockType* bt = getNextBlockPtr(GetRoot());
		while (bt)
		{
			if (bt->IsContain(et))
			{
				if (bt->Detach(et, elementCount_))
				{
					return true;
				}
				else
				{
					ASSERT(0);
				}
				break;
			}
			bt = getNextBlockPtr(bt);
		}
		ASSERT(0);
		return false;
	}
	bool Detach(IndexType index)
	{
		ASSERT(index >= 0);
		ASSERT(index < BLOCK_VAULT_BLOCK_LIMIT * N);
		ASSERT(elementCount_ > 0);

		BlockType* bt = GetRoot();
		while (index >= N)
		{
			index -= N;
			if (bt->GetNext())
			{
				bt = getNextBlockPtr(bt);
			}
			else
			{
				ASSERT(0);
				return false;
			}
		}
		return bt->Detach(index, elementCount_);
	}
	ElementType* Get(IndexType index)
	{
		ASSERT(index >= 0);
		if (index > BLOCK_VAULT_BLOCK_LIMIT * N)
			return nullptr;
		if (index < N)
		{
			return GetRoot()->Get(index);
		}
		BlockType* bt = getNextBlockPtr(GetRoot());
		while (bt)
		{
			index -= N;
			if (index < N)
			{
				return bt->Get(index);
			}
			bt = getNextBlockPtr(bt);
		}
		ASSERT(0);
		return nullptr;
	}
	ElementType* operator[](IndexType index)
	{
		return Get(index);
	}
	bool GetIndex(ElementType* element, IndexType& index)
	{
		index = 0;
		BlockType* bt = GetRoot();
		while (bt)
		{
			if (bt->GetIndex(element, index))
			{
				return true;
			}
			else
			{
				if (bt->GetNext())
				{
					bt = getNextBlockPtr(bt);
					index += N;
				}
				else
				{
					break;
				}
			}
		}
		return false;
	}
	uint16 GetIndex(ElementType* element)
	{
		IndexType index;
		if (GetIndex(element, index))
		{
			return index;
		}
		return (IndexType)-1;
	}
	size_t Capacity()const
	{
		return blockCount_*N;
	}
	bool IsFull()const
	{
		return Capacity() == elementCount_;
	}
	size_t GetElementCount()const
	{
		return elementCount_;
	}
	size_t Count()const
	{
		return elementCount_;
	}
	Iterator Begin()
	{
		return Iterator(GetRoot());
	}
	Iterator End()
	{
		return Iterator();
	}
	void Clear()
	{
		Clean();
	}
	void Clean()
	{
		BlockType* block = GetRoot();
		while (block)
		{
			block->Clean();
			if (!block->GetNext())
			{
				break;
			}
			else
			{
				block = getNextBlockPtr(block);
			}
		}
		freeBlock();
	}
	void Flush()
	{
		BlockType* block = GetRoot();
		while (block)
		{
			FactoryType::instance().flush(block, 0, sizeof(BlockType));
			if (!block->GetNext())
			{
				break;
			}
			else
			{
				block = getNextBlockPtr(block);
			}
		}
	}

	template<typename CompareFunctor, typename ParaType>
	ElementType* Find(ParaType para)
	{
		CompareFunctor functor;
		for (auto iter = Begin(); iter != End(); ++iter)
		{
			if (functor(iter.Get(), para))
			{
				return iter.Get();
			}
		}
		return nullptr;
	}
private:
	void freeBlock()
	{
		blockCount_ = 1;
		elementCount_ = 0;
		BlockType * block = getNextBlockPtr(GetRoot());
		while (block)
		{
			BlockType* next = getNextBlockPtr(block);
			block = next;
		}
	}
	BlockType* newBlock()
	{
		ASSERT(blockCount_ < BLOCK_VAULT_BLOCK_LIMIT);
		BlockType* bt = new(zone_->New(sizeof(BlockType)))BlockType();;
		if (bt)
		{
			++blockCount_;
		}
		if (!GetRoot()->GetNext())
		{
			setNextBlockPtr(GetRoot(), bt);
			return bt;
		}
		BlockType * block = getNextBlockPtr(GetRoot());
		while (block)
		{
			if (!block->GetNext())
			{
				setNextBlockPtr(block, bt);
				break;
			}
			else
			{
				block = getNextBlockPtr(block);
			}
		}
		return bt;
	}
	inline BlockType* getNextBlockPtr(BlockType* prev)
	{
		return prev->GetNext();
	}
	inline bool       setNextBlockPtr(BlockType* prev, BlockType* next)
	{
		if (next)
		{
			prev->SetNext(next);
			return true;
		}
		return false;
	}
	BlockType* GetRoot()
	{
		return head_;
	}
private:
	size_t						blockCount_;
	size_t						elementCount_;
	BlockType*					head_;
	ZoneMemory*					zone_;
};

#endif // !ZoneVault_H_

