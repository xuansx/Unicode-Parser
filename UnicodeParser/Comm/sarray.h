#ifndef sarray_h_sim_
#define sarray_h_sim_

template<typename ElementT,typename LengthT = int,size_t CAPACITY = 8>
class static_array
{
public:
	typedef ElementT			element_type;
	typedef LengthT				length_type;

	typedef element_type         value_type;
	typedef element_type*        iterator;
	typedef const element_type*  const_iterator;
	typedef element_type&        reference;
	typedef const element_type&  const_reference;
	typedef static_array<ElementT,LengthT,CAPACITY>	this_type;

	static_array()
		:length_(0)
	{
		clear();
	}
	static_array(length_type len)
		:length_(len)
	{
		clear();
		ASSERT(len <= CAPACITY);
	}
	static_array(static_array&other)
		:length_(other.length())
	{
		memcpy(elems,other.elems,other.length()*sizeof(element_type));
	}
	// iterator support
	iterator        begin()       { return elems; }
	const_iterator  begin() const { return elems; }
	const_iterator cbegin() const { return elems; }

	iterator        end()       { return elems+length_; }
	const_iterator  end() const { return elems+length_; }
	const_iterator cend() const { return elems+length_; }
	iterator		back()
	{
		if (!empty())
		{
			return elems[length_-1];
		}
		return end();
	}
	length_type length()const
	{
		return length_;
	}
	length_type size()const
	{
		return length_;
	}
	length_type capacity()const
	{
		return CAPACITY;
	}
	void make_full()
	{
		length_ = CAPACITY;
	}
	void resize(length_type len = 0 )
	{
		if (len == 0)
		{
			length_ = CAPACITY;
		}else
		{
			length_ = len;
		}
	}
	void length(length_type len)
	{
		if(len > CAPACITY) throw "out of range.";
		length_ = len;
	}
	void push_back(const element_type& e)
	{
		if (length_ < CAPACITY)
		{
			elems[length_++] = e;
		}else
		{
			throw "out of range.";
		}
	}
	void pop_back(const element_type& e)
	{
		if (length_ > 0)
		{
			elems[length_ - 1] = e;
			--length_;
		}
	}
	void push_front(const element_type& e)
	{
		shift_right(0);
		elems[0] = e;
	}
	element_type& pop_front()
	{
		shift_left(0);
		if (length_ > 1)
		{
			return elems[0];
		}else
		{
			throw "out of range.";
		}
	}
	element_type& pop_back()
	{
		if (length_ > 0)
		{
			return elems[--length_];
		}else
		{
			throw "out of range.";
		}
	}
	void insert_unordered(length_type index,const element_type& e)
	{
		if (index < CAPACITY)
		{
			if (length_ < index)
			{
				length_ = index;
			}
			elems[index] = e;
		}else
		{
			throw "out of range.";
		}
	}
	void erase(length_type index)
	{
		shift_left(index);
	}

	void shift_left(length_type begin)
	{
		for (length_type i = begin ; (i + 1) < length_ ;i++)
		{
			elems[i] = elems[i + 1];
		}
		--length_;
	}
	void shift_right(length_type begin)
	{
		if(begin >= 0 && (length_  < CAPACITY))
		for (length_type i = length_; i > begin ;i--)
		{
			elems[i] = elems[i - 1];
		}
		++length_;
	}
	reference operator[](length_type i)
	{
		if(i >= length_) throw "out of range.";
		return elems[i];
	}
	reference at(length_type i)
	{
		if(i >= length_) throw "out of range.";
		return elems[i];
	}
	element_type* at_ptr(length_type i)
	{
		if (i >= length_)
		{
			return nullptr;
		}
		return &elems[i];
	}
	void reset()
	{
		length_ = 0;
		clear();
	}
	bool empty()const
	{
		return length_ == 0;
	}
	bool full() const
	{
		return length_ == capacity();
	}
	/*
	struct TestRole
	{
		int32 id(){return 0;};
	};
	struct comp_role_oper 
	{
		inline bool operator()(TestRole& r,int32 n)
		{
			if(r.id() == n)
			{
				return true;
			}
			return false;
		}
	};
	void test()
	{
		static_array<TestRole>	arr;
		int id = 12;
		arr.find<comp_role_oper,int32>(id);
	}
	*/
	template<typename CompareOper,typename CompareParameter>
	length_type find(CompareParameter& param)
	{
		CompareOper oper;
		for (length_type i = 0; i < length_; i++)
		{
			if (oper(elems[i],param))
			{
				return i;
			}
		}
		return CAPACITY;
	}
protected:
	inline void clear()
	{
		memset(&elems,0,sizeof(elems));
	}
private:
	length_type					length_;
	element_type				elems[CAPACITY];
};

#endif // sarray_h__

