//借鉴tinyxml的树的结构特点
#pragma once
#ifndef TREE_H_SAM_
#define TREE_H_SAM_
#include "object_pool.hpp"

template<typename T>
struct NodeT :public T
{
public:
	typedef T					ElementType;
	typedef NodeT<ElementType>	NodeType;
	NodeT() :parent_(nullptr),nextSibling_(nullptr), firstChild_(nullptr), childrenCount_(0)
	{}
	template<typename P0>
	NodeT(P0 p0)
		: ElementType(p0), 
		parent_(nullptr), nextSibling_(nullptr), firstChild_(nullptr), childrenCount_(0)
	{}
	template<typename P0, typename P1>
	NodeT(P0 p0,P1 p1)
		: ElementType(p0,p1), 
		parent_(nullptr), nextSibling_(nullptr), firstChild_(nullptr), childrenCount_(0)
	{}
	void AddSibling(NodeType* sibling)
	{
		if (!nextSibling_)
		{
			nextSibling_ = sibling;
			return;
		}
		NodeType* next = nextSibling_;
		while (next->nextSibling_)
		{
			next = next->nextSibling_;
		}
		next->nextSibling_ = sibling;
	}
	bool DelSibling(NodeType* sibling)
	{
		if (!nextSibling_ || !sibling)
		{
			return false;
		}
		if (nextSibling_ == sibling)
		{
			nextSibling_ = nextSibling_->nextSibling_;
			return true;
		}
		NodeType* next = nextSibling_->nextSibling_;
		NodeType* prev = nextSibling_;
		while (next)
		{
			if (next == sibling)
			{
				break;
			}
			prev = next;
			next = next->nextSibling_;
		}
		if (next)
		{
			prev->nextSibling_ = next->nextSibling_;
			return true;
		}
		return false;
	}
	NodeType* GetSibling()
	{
		return nextSibling_;
	}
	NodeType* GetFirstChild()
	{
		return firstChild_;
	}
	void AddChild(NodeType* child)
	{
		if (!firstChild_)
		{
			firstChild_ = child;
			childrenCount_ = 1;
			child->parent_ = this;
			return;
		}
		firstChild_->AddSibling(child);
		child->parent_ = this;
		++childrenCount_;
	}
	bool DelChild(NodeType* child)
	{
		if (!firstChild_ || !child)
		{
			return false;
		}
		ASSERT(childrenCount_ > 0);
		if (firstChild_ == child)
		{
			firstChild_ = firstChild_->nextSibling_;
			--childrenCount_;
			return true;
		}
		if (firstChild_->DelSibling(child))
		{
			--childrenCount_;
			return true;
		}
		return false;
	}
	size_t GetChildrenCount()const
	{
		return childrenCount_;
	}
	bool IsChildless()const
	{
		return childrenCount_ == 0;
	}
	NodeType* GetParent()
	{
		return parent_;
	}
	void SetParentLight(NodeType* parent)
	{
		parent_ = parent;
	}
private:
	NodeType* parent_;
	NodeType* nextSibling_;
	NodeType* firstChild_;
	size_t childrenCount_;
};

template<typename T>
struct NodeDataHolderT
{
public:
	typedef T					ElementType;
	void SetHoldData(ElementType& data)
	{
		data_ = data;
	}
	ElementType& GetHoldData()
	{
		return data_;
	}
private:
	ElementType		data_;
};

template<typename T, typename PoolType>
class TreeBaseT
{
public:
	typedef T								ElementType;
	typedef NodeT<ElementType>				NodeType;
	typedef PoolType						NodePool;
public:
	TreeBaseT(NodePool& pool)
		:head_(nullptr), size_(0), pool_(pool)
	{
	}
	~TreeBaseT()
	{
		if (head_)
		{
			Destroy(head_);
			head_ = nullptr;
		}
	}
	NodeType* CreateRoot()
	{
		head_ = Create();
		return head_;
	}
	NodeType* GetRoot()
	{
		return head_;
	}
	void ResetRoot(NodeType* root)
	{
		if (head_)
		{
			Destroy(head_);
		}
		head_ = root;
	}
	NodeType* Create()
	{
		NodeType* node = pool_.construct();
		if (node)
		{
			++size_;
		}
		return node;
	}
	template<typename P0>
	NodeType* Create(P0 p0)
	{
		NodeType* node = pool_.construct(p0);
		if (node)
		{
			++size_;
		}
		return node;
	}
	template<typename P0, typename P1>
	NodeType* Create(P0 p0, P1 p1)
	{
		NodeType* node = pool_.construct(p0, p1);
		if (node)
		{
			++size_;
		}
		return node;
	}
	template<typename P0, typename P1, typename P2>
	NodeType* Create(P0 p0, P1 p1,P2 p2)
	{
		NodeType* node = pool_.construct(p0, p1, p2);
		if (node)
		{
			++size_;
		}
		return node;
	}
	template<typename P0, typename P1, typename P2, typename P3>
	NodeType* Create(P0 p0, P1 p1, P2 p2,P3 p3)
	{
		NodeType* node = pool_.construct(p0, p1, p2, p3);
		if (node)
		{
			++size_;
		}
		return node;
	}
	void Destroy(NodeType* node)
	{
		ASSERT(node != nullptr);
		ASSERT(size_ > 0);
		--size_;
		NodeType* child = node->GetFirstChild();
		while (child)
		{
			NodeType* n = child;
			child = child->GetSibling();
			Destroy(n);
		}
		pool_.destroy(node);
	}
	void DestroyOne(NodeType* node)
	{
		ASSERT(node != nullptr);
		ASSERT(size_ > 0);
		--size_;
		pool_.destroy(node);
	}
	size_t Size()const
	{
		return size_;
	}
	void Clean()
	{
		if (head_)
		{
			Destroy(head_);
		}
		ASSERT(size_ == 0);
	}
	void CleanOnly()
	{
		if (head_)
		{
			head_ = nullptr;
			size_ = 0;
		}
		ASSERT(size_ == 0);
	}
private:
	size_t		size_;
	NodeType*	head_;
	NodePool&	pool_;
};

template<typename T, size_t POOL_BLOCK_SZ>
struct TreeHolderPool
{
	typedef T															ElementType;
	typedef NodeDataHolderT<ElementType>								HolderType;
	typedef typename OBJECT_POOL_TYPE(NodeT<HolderType>, POOL_BLOCK_SZ)	PoolType;
	static PoolType& GetPool()
	{
		static PoolType pool;
		return pool;
	}
};
template<typename Traits>
class TreeHolder:public TreeBaseT<typename Traits::HolderType, typename Traits::PoolType>
{
public:
	typedef TreeBaseT<typename Traits::HolderType, typename Traits::PoolType>	BaseType;
	TreeHolder()
		:BaseType(Traits::GetPool())
	{}
};

template<typename T,size_t POOL_BLOCK_SZ = 255>
class Tree:public TreeBaseT<T, typename OBJECT_POOL_TYPE(NodeT<T>, POOL_BLOCK_SZ)>
{
public:
	typedef T												ElementType;
	typedef NodeT<ElementType>								NodeType;
	typedef OBJECT_POOL_TYPE(NodeType, POOL_BLOCK_SZ)		NodePool;
	typedef TreeBaseT<T, NodePool>							TreeBase;
public:
	Tree()
		:TreeBase(pool_)
	{
	}
private:
	NodePool	pool_;
};


#endif // !TREE_H_SAM_

