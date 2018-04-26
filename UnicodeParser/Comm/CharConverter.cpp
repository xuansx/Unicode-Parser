#include "CharConverter.h"
#include "FileIO.h"
#include "sstring.hpp"

#include <vector>

CharConverter::CharConverter()
	:convert_gbk_unicode(new std::codecvt<wchar_t, char, std::mbstate_t>("CHS"))
{
}


CharConverter::~CharConverter()
{
}
bool CharConverter::Load(const char* filename)
{
	memset(gbkIndexUnicodeArray_, 0, sizeof(gbkIndexUnicodeArray_));
	memset(uniIndexGBKArray_, 0, sizeof(uniIndexGBKArray_));

	BufferFile	reader;
	if (!reader.ReadToBuffer(filename))
	{
		return false;
	}
	wchar_t* begin = (wchar_t*)reader.GetBuffer();
	size_t	 length = reader.GetLength() / 2;
	wchar_t* p = begin;
	if (*p == 0xFEFF)
	{
		++p;
	}
	unsigned short code[2] = { 0,0 };
	int index = 0;
	while (*p)
	{
		if ('0'<=(*p) && (*p) <= '9')
		{
			code[index] *= 16;
			code[index] += *p - '0';
		}
		else if ('A' <= (*p) && (*p) <= 'F')
		{
			code[index] *= 16;
			code[index] += (*p - 'A' + 10);
		}
		else if (*p == '\t')
		{
			++index;
		}
		else if(*p == '\n')
		{
			code[0] = 0;
			code[1] = 0;
			index = 0;
		}
		else if(*p == '\r')
		{
			if (code[0] >= GBK_BEGIN_CODE && code[0] <= GBK_END_CODE)
			{
				gbkIndexUnicodeArray_[code[0] - GBK_BEGIN_CODE] = code[1];
				uniIndexGBKArray_[code[1]] = code[0];
			}
		}
		++p;
	}	
	return true;
}

