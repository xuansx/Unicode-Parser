#pragma once
#ifndef StrArrayUtility_H_
#define StrArrayUtility_H_
#include <string>
/*
min_begin is none as default failed result;
*/
template<typename E,typename S,int M,int N>
struct StrArrayReflectIndex
{
public:
	typedef S		StringType;
	typedef E		EnumType;
	StrArrayReflectIndex(const StringType* strArray)
		:strArray_(strArray)
	{
	}
	inline EnumType GetIndex(const StringType& name)
	{
		for (int i = 0; i < N; i++)
		{
			if (strArray_[i] == name)
			{
				return (EnumType)i;
			}
		}
		return (EnumType)M;
	}
	inline const StringType& GetString(int index)
	{
		if (index < N)
		{
			return strArray_[index];
		}
		return strArray_[M];
	}
private:
	const StringType* strArray_;
};

#endif // !StrArrayUtility_H_
