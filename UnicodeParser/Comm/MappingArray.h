#pragma once
#ifndef MappingArray_H__
#define MappingArray_H__

template<typename T>
struct MappingArray
{
public:
	typedef T			ElementType;
	MappingArray()
	{
		//nothing
	}
	static size_t SizeOf(size_t count)
	{
		size_t len = sizeof(length_);
		len += sizeof(ElementType) * count;
		return len;
	}
	size_t SizeOfMy()
	{
		return SizeOf(length());
	}
	void SetLength(size_t len)
	{
		length_ = len;
	}
	size_t length()const
	{
		return length_;
	}
	ElementType* begin()
	{
		return &data_[0];
	}
	ElementType* end()
	{
		return begin() + length_;
	}
	ElementType* operator[](size_t index)
	{
		return at(index);
	}
	ElementType* at(size_t index)
	{
		if (index >= length_)
		{
			return nullptr;
		}
		return begin() + index;
	}
private:
	size_t		length_;
	ElementType	data_[1];
};


template<typename T>
MappingArray<T>* InitMappingArray(void* mem)
{
	if (mem)
	{
		return static_cast<MappingArray<T>>(mem);
	}
	return nullptr;
}

#endif // !MappingArray_H__

