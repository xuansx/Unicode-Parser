#pragma once
#ifndef LinkedList_H_
#define LinkedList_H_

template<typename OT>
struct LinkedNode:public OT
{
public:
	typedef OT						ObjectType;
	typedef LinkedNode<ObjectType>	NodeType;
	LinkedNode()
		:next_(nullptr), prev_(nullptr)
	{}
	template<typename P0>
	LinkedNode(P0 p0)
		: ObjectType(p0), next_(nullptr), prev_(nullptr)
	{}
	template<typename P0, typename P1>
	LinkedNode(P0 p0,P1 p1)
		: ObjectType(p0,p1), next_(nullptr), prev_(nullptr)
	{}
	void SetNext(NodeType*const next)
	{
		next_ = next;
	}
	NodeType* GetNext()const
	{
		return next_;
	}
	void SetPrev(NodeType*const prev)
	{
		prev_ = prev;
	}
	NodeType* GetPrev()const
	{
		return prev_;
	}
private:
	NodeType*		next_;
	NodeType*		prev_;
};


template<typename OT>
struct LinkedNodeProto
{
public:
	typedef OT						ObjectType;
	typedef ObjectType				NodeType;
	LinkedNodeProto()
		:next_(nullptr), prev_(nullptr)
	{}
	void SetNext(NodeType*const next)
	{
		next_ = next;
	}
	NodeType* GetNext()const
	{
		return next_;
	}
	void SetPrev(NodeType*const prev)
	{
		prev_ = prev;
	}
	NodeType* GetPrev()const
	{
		return prev_;
	}
private:
	NodeType*		next_;
	NodeType*		prev_;
};

template<typename OT>
class LinkedListProto
{
public:
	typedef OT		NodeType;
	LinkedListProto()
		:head_(nullptr), tail_(nullptr)
	{

	}
	template <typename Func>
	NodeType* TestFindFirst()
	{
		typedef Func		FunctionType;
		NodeType* node = head_;
		while (node)
		{
			if (node->FunctionType())
			{
				return node;
			}
			if (node == tail_)
			{
				break;
			}
			node = node->GetNext();
		}
		return nullptr;
	}
	template <typename Func>
	void DoForEach(Func func)
	{
		NodeType* node = head_;
		while (node)
		{
			func(node);
			if (node == tail_)
			{
				break;
			}
			node = node->GetNext();
		}
	}
	template <typename Func>
	void CallForEach()
	{
		typedef Func		FunctionType;
		NodeType* node = head_;
		while (node)
		{
			node->FunctionType();
			if (node == tail_)
			{
				break;
			}
			node = node->GetNext();
		}
	}
	template <typename Func, typename P0>
	void CallForEach(P0 p0)
	{
		typedef Func		FunctionType;
		NodeType* node = head_;
		while (node)
		{
			node->FunctionType(p0);
			if (node == tail_)
			{
				break;
			}
			node = node->GetNext();
		}
	}
	template <typename Func, typename P0, typename P1>
	void CallForEach(P0 p0, P1 p1)
	{
		typedef Func		FunctionType;
		NodeType* node = head_;
		while (node)
		{
			node->FunctionType(p0,p1);
			if (node == tail_)
			{
				break;
			}
			node = node->GetNext();
		}
	}
	NodeType* GetHead()
	{
		return head_;
	}
	NodeType* GetTail()
	{
		return tail_;
	}
	void Insert(NodeType* afterThisNode, NodeType* node)
	{
		if (afterThisNode == tail_)
		{
			PushBack(node);
			return;
		}
		NodeType* next = afterThisNode->GetNext();
		afterThisNode->SetNext(node);
		node->SetPrev(afterThisNode);
		node->SetNext(next);
	}
	void Remove(NodeType* node)
	{
		if (!node)
		{
			return;
		}
		if (!tail_)
		{
			return;
		}
		if (node == tail_)
		{
			PopBack();
		}
		if (node == head_)
		{
			PopFront();
		}
		node->GetPrev()->SetNext(node->GetNext());
		node->GetNext()->SetPrev(node->GetPrev());
	}
	void PushFront(NodeType* node)
	{
		if (head_)
		{
			head_->SetPrev(node);
			node->SetNext(head_);
			head_ = node;
		}
		else
		{
			addFirstNode(node);
		}
	}
	NodeType* PopFront(NodeType* node)
	{
		if (!head_)
		{
			return nullptr;
		}
		NodeType* node = head_;
		head_ = head_->GetNext();
		if (!head_)
		{
			tail_ = nullptr;
		}
		return node;
	}
	void PushBack(NodeType* node)
	{
		if (tail_)
		{
			tail_->SetNext(node);
			node->SetPrev(tail_);
			tail_ = node;
		}
		else
		{
			addFirstNode(node);
		}
	}
	NodeType* PopBack()
	{
		if (!tail_)
		{
			return nullptr;
		}
		NodeType* node = tail_;
		tail_ = tail_->GetPrev();
		if (!tail_)
		{
			head_ = nullptr;
		}
		return node;
	}
	void Clean()
	{
		head_ = nullptr;
		tail_ = nullptr;
	}
private:
	void addFirstNode(NodeType* node)
	{
		head_ = node;
		tail_ = node;
	}
private:
	NodeType*		head_;
	NodeType*		tail_;
};
#endif // !LinkedList_H_
