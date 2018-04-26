#pragma once
#ifndef CharConverter_H_
#define CharConverter_H_

#include "Singleton.h"

#include <string>
#include <codecvt>

class CharConverter:public SingletonStaticT<CharConverter>
{
public:
	static const unsigned short GBK_BEGIN_CODE	= 0x8140;
	static const unsigned short GBK_END_CODE	= 0xFEA0;
	CharConverter();
	~CharConverter();
	bool Load(const char* filename);
	wchar_t GetUnicode(wchar_t gbk)
	{
		if (gbk >= GBK_BEGIN_CODE && gbk <= GBK_END_CODE)
		{
			return gbkIndexUnicodeArray_[gbk-GBK_BEGIN_CODE];
		}
		return 0;
	}
	wchar_t GetGBK(wchar_t unicode)
	{
		return uniIndexGBKArray_[unicode];
	}
	unsigned char* UnicodeToUtf8(char32_t unic)
	{
		utf8[0] = 0;
		unsigned char* pOutput = utf8;
		if (unic <= 0x0000007F)
		{
			// * U-00000000 - U-0000007F:  0xxxxxxx  
			*pOutput = (unic & 0x7F);
			utf8[1] = 0;
			utf8[7] = 1;
		}
		else if (unic >= 0x00000080 && unic <= 0x000007FF)
		{
			// * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx  
			*(pOutput + 1) = (unic & 0x3F) | 0x80;
			*pOutput = ((unic >> 6) & 0x1F) | 0xC0;
			utf8[2] = 0;
			utf8[7] = 2;
		}
		else if (unic >= 0x00000800 && unic <= 0x0000FFFF)
		{
			// * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx  
			*(pOutput + 2) = (unic & 0x3F) | 0x80;
			*(pOutput + 1) = ((unic >> 6) & 0x3F) | 0x80;
			*pOutput = ((unic >> 12) & 0x0F) | 0xE0;
			utf8[3] = 0;
			utf8[7] = 3;
		}
		else if (unic >= 0x00010000 && unic <= 0x001FFFFF)
		{
			// * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
			*(pOutput + 3) = (unic & 0x3F) | 0x80;
			*(pOutput + 2) = ((unic >> 6) & 0x3F) | 0x80;
			*(pOutput + 1) = ((unic >> 12) & 0x3F) | 0x80;
			*pOutput = ((unic >> 18) & 0x07) | 0xF0;
			utf8[4] = 0;
			utf8[7] = 4;
		}
		else if (unic >= 0x00200000 && unic <= 0x03FFFFFF)
		{
			// * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
			*(pOutput + 4) = (unic & 0x3F) | 0x80;
			*(pOutput + 3) = ((unic >> 6) & 0x3F) | 0x80;
			*(pOutput + 2) = ((unic >> 12) & 0x3F) | 0x80;
			*(pOutput + 1) = ((unic >> 18) & 0x3F) | 0x80;
			*pOutput = ((unic >> 24) & 0x03) | 0xF8;
			utf8[5] = 0;
			utf8[7] = 5;
		}
		else if (unic >= 0x04000000 && unic <= 0x7FFFFFFF)
		{
			// * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
			*(pOutput + 5) = (unic & 0x3F) | 0x80;
			*(pOutput + 4) = ((unic >> 6) & 0x3F) | 0x80;
			*(pOutput + 3) = ((unic >> 12) & 0x3F) | 0x80;
			*(pOutput + 2) = ((unic >> 18) & 0x3F) | 0x80;
			*(pOutput + 1) = ((unic >> 24) & 0x3F) | 0x80;
			*pOutput = ((unic >> 30) & 0x01) | 0xFC;
			utf8[6] = 0;
			utf8[7] = 6;
		}
		else
		{
			return nullptr;
		}

		return utf8;
	}
	//uni-->utf8,strlen of result utf8;
	unsigned char GetUtf8StrLen()const
	{
		return utf8[7];
	}
	char32_t Utf8ToUnicode(const unsigned char* pInput)
	{
		//assert(pInput != NULL && Unic != NULL);

		// b1 表示UTF-8编码的pInput中的高字节, b2 表示次高字节, ...  
		char b1, b2, b3, b4, b5, b6;

		int utfbytes = 0;
		unicode = 0;
		unsigned char mask = 0xC;
		for (int i = 0; i < 4; ++i)
		{
			if (((*pInput) & mask) > 0)
			{
				utfbytes = i+2;
				break;
			}
			mask >>= 1;
		}
		unsigned char *pOutput = (unsigned char *)&unicode;

		switch (utfbytes)
		{
		case 0:
			*pOutput = *pInput;
			utfbytes += 1;
			break;
		case 2:
			b1 = *pInput;
			b2 = *(pInput + 1);
			if ((b2 & 0xE0) != 0x80)
				return 0;
			*pOutput = (b1 << 6) + (b2 & 0x3F);
			*(pOutput + 1) = (b1 >> 2) & 0x07;
			break;
		case 3:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80))
				return 0;
			*pOutput = (b2 << 6) + (b3 & 0x3F);
			*(pOutput + 1) = (b1 << 4) + ((b2 >> 2) & 0x0F);
			break;
		case 4:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			b4 = *(pInput + 3);
			if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
				|| ((b4 & 0xC0) != 0x80))
				return 0;
			*pOutput = (b3 << 6) + (b4 & 0x3F);
			*(pOutput + 1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
			*(pOutput + 2) = ((b1 << 2) & 0x1C) + ((b2 >> 4) & 0x03);
			break;
		case 5:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			b4 = *(pInput + 3);
			b5 = *(pInput + 4);
			if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
				|| ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80))
				return 0;
			*pOutput = (b4 << 6) + (b5 & 0x3F);
			*(pOutput + 1) = (b3 << 4) + ((b4 >> 2) & 0x0F);
			*(pOutput + 2) = (b2 << 2) + ((b3 >> 4) & 0x03);
			*(pOutput + 3) = (b1 << 6);
			break;
		case 6:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			b4 = *(pInput + 3);
			b5 = *(pInput + 4);
			b6 = *(pInput + 5);
			if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
				|| ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)
				|| ((b6 & 0xC0) != 0x80))
				return 0;
			*pOutput = (b5 << 6) + (b6 & 0x3F);
			*(pOutput + 1) = (b5 << 4) + ((b6 >> 2) & 0x0F);
			*(pOutput + 2) = (b3 << 2) + ((b4 >> 4) & 0x03);
			*(pOutput + 3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
			break;
		default:
			return 0;
			break;
		}

		return unicode;
	}

	inline std::string& GBKToUtf8(std::wstring const &strGBK)
	{
		return convert_gbk_unicode.to_bytes(strGBK);
	}

	inline std::wstring& Utf8ToGBK(std::string const &strUtf8)
	{
		return convert_gbk_unicode.from_bytes(strUtf8);
	}
	inline std::string& UnicodeToUtf8(std::wstring& unicode)
	{
		return convert_utf8_unicode.to_bytes(unicode);
	}
	inline std::wstring& Utf8ToUnicode(std::string& utf8)
	{
		return convert_utf8_unicode.from_bytes(utf8);
	}
public:
	std::wstring_convert<std::codecvt_utf8<wchar_t>>	convert_utf8_unicode;
	//GBK<->Unicode转换器
	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>
		convert_gbk_unicode;
private:
	union {
		char32_t			unicode;
		unsigned char		utf8[8];
	};
	wchar_t		gbkIndexUnicodeArray_[0xFEA0-0x8140];
	wchar_t		uniIndexGBKArray_[0xFFFF];
};
#endif // !GBKUnicodeConverter_H_
