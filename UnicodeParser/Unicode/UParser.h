//most unicode data file standard parser (escape NamesList.txt)
#pragma once
#ifndef UParser_H__
#define UParser_H__

#include "FileIO.h"
#include "signature.hpp"
#include "sutility.hpp"

class ParserBase
{
public:
	const char* NextLine(const char* source, signature& line)
	{
		bool isCommentBegin = false;
		line.reset(source);
		while (*source)
		{
			if (isEndLine(*source))
			{
				++source;
				break;
			}
			if (*source == '#')
			{
				isCommentBegin = true;
			}
			if (!isCommentBegin)
			{
				line.grow();
			}
			++source;
		}
		return source;
	}
	bool isEndLine(int c)const
	{
		return c == '\r' || c == '\n';
	}
};
template<typename T, size_t C>
class DataLineParser :public ParserBase
{
public:
	typedef T		DataType;
	DataLineParser()
		:lineCount_(0), readPtr_(nullptr)
	{}
	bool Load(const char* dataFileName)
	{
		if (!bufFile_.ReadToBuffer(dataFileName))
		{
			return false;
		}
		readPtr_ = bufFile_.GetBuffer();
		return true;
	}
	bool Parse()
	{
		signature	line;
		lineCount_ = 0;
		readPtr_ = bufFile_.GetBuffer();
		while (*readPtr_)
		{
			readPtr_ = NextLine(readPtr_, line);
			if (!line.empty())
			{
				if (!lineToData(line))
				{
					return false;
				}
			}
		}
		return true;
	}
	DataType* GetValue(uint32_t index)
	{
		if (index >= lineCount_)
		{
			return nullptr;
		}
		return &values_[index];
	}
	uint32_t GetLineCount()const
	{
		return lineCount_;
	}
private:
	bool lineToData(signature& line)
	{
		lineSparator_.reset_source(line);
		DataType* value = getUnuseValue();
		if (!value)
		{
			return false;
		}
		return value->Init(lineSparator_);
	}
	DataType* getUnuseValue()
	{
		if (lineCount_ >= C)
		{
			return nullptr;
		}
		return &values_[lineCount_++];
	}
private:
	const char*			readPtr_;
	uint32_t			lineCount_;
	DataType			values_[C];
	BufferFile			bufFile_;
	sigseparator_8		lineSparator_;
};

#endif // !UParser_H__
