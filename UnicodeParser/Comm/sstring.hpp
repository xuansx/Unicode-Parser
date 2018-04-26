#ifndef sstring_h_sim
#define sstring_h_sim

#include <string>
#include <stdio.h>
#include <stdarg.h>
#include "signature.hpp"

#ifndef ASSERT
#if defined _DEBUG
#include <assert.h>
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif
#endif

template<typename CharType,typename Traits,size_t CAPACITY = 8>
class static_string_base
{
public:
	typedef		static_string_base<CharType,Traits,CAPACITY>	MyType;
	typedef		Traits											traits_type;
	typedef		std::basic_string<CharType, Traits, std::allocator<CharType> > std_string;
	static_string_base()
		:length_(0)
	{
		data_[length_] = 0;
	}
	static_string_base(const CharType* str)
		:length_(0)
	{
		if (str!=NULL)
		{
			length_ = Traits::length(str);
			init(str);
		}else
		{
			data_[0] = 0;
		}
	}
	static_string_base(const CharType* str,size_t len)
		:length_(len)
	{
		ASSERT(len < CAPACITY);
		if (str != NULL)
		{
			init(str);
		}else
		{
			length_ = 0;
			data_[0]= 0;
		}
	}
	static_string_base(const std_string& str)
		:length_(str.length())
	{
		init(str.c_str());
	}
	static_string_base(const MyType& other)
		:length_(other.length_)
	{
		init(other.data_);
	}
	MyType& operator=(const MyType& other)
	{
		reset();
		copyFrom(other.data_,other.length_);
		return *this;
	}
	MyType& operator=(const std_string& other)
	{
		reset();
		copyFrom(other.c_str(), other.length());
		return *this;
	}
	MyType& operator=(const CharType* str)
	{
		if (str)
		{
			length_ = Traits::length(str);
			init(str);
		}else
		{
			clear();
		}
		return *this;
	}
	MyType& operator=(CharType ch)
	{
		data_[0] = ch;
		length_	 = 1;
		data_[length_] = 0;
		return *this;
	}
	MyType&	assign(const CharType* str, size_t len)
	{
		if (str)
		{
			length_ = len;
			init(str);
		}
		else
		{
			clear();
		}
		return *this;
	}
	MyType&	assign(const CharType* str)
	{
		return assign(str, Traits::length(str));
	}
	MyType& assign(const std_string& other)
	{
		reset();
		return assign(other.c_str(), other.length());
	}
	inline MyType sub_str(size_t begin_index,size_t count)
	{
		if (begin_index >= length_)
		{
			begin_index = length_;
			count = 0;
		}
		if (begin_index + count > length_)
		{
			count = length_ - begin_index;
		}
		return MyType(begin()+begin_index,count);
	}
	bool equal(const MyType& other)const
	{
		if (length() == other.length())
		{
			return Traits::compare(begin(),other.begin(),length()) == 0;
		}
		return false;
	}
	bool equal(const CharType* str)const
	{
		if (!str)
		{
			return false;
		}
		return equal(str,Traits::length(str));
	}
	bool equal(const CharType* str,size_t l)const
	{
		if (!str)
		{
			return false;
		}
		if (length() == l)
		{
			return Traits::compare(begin(),str,length()) == 0;
		}
		return false;
	}
	template<typename OtherType>
	MyType& operator=(OtherType& other)
	{
		reset();
		size_t len = other.length();
		if (capacity() <= other.length())
		{
			len = capacity();
		}
		copyFrom(other.begin(),len);
		return *this;
	}
	int compare(const MyType& other)const
	{
		return compare(other.begin(),other.length());
	}
	int compare(const CharType* str,size_t l)const
	{
		size_t len = l;
		if (len > length())
		{
			len = length();
		}
		int cmpr = Traits::compare(str,begin(),len);
		if ((cmpr == 0) && (l != length()))
		{
			if (length() < l)
			{
				return -1;
			}else
			{
				return 1;
			}
		}
		return cmpr;
	}
	int compare(const CharType* str)const
	{
		return compare(str,Traits::length(str));
	}

	const CharType* begin()const
	{
		return data_;
	}
	const CharType* end()const
	{
		return begin()+length_;
	}
	CharType* begin()
	{
		return data_;
	}
	const CharType* c_str()const
	{
		return begin();
	}
	CharType* c_str()
	{
		return begin();
	}

	CharType* end()
	{
		return begin()+length_;
	}
	int printf(const char* format,...)
	{
		int		res = 0;
		va_list args;
		va_start(args,format);
#ifdef _WIN32
		res = vsprintf_s(end(),capacity()-length(),format,args);
#else
		vsprintf(end(),format,args);
#endif
		va_end(args);
		if (res >=(int)capacity())
		{
			res = int(capacity() - 1);
		}
		data_[res] = 0;
		length_    = res + length_;
		return res;
	}
	CharType at(size_t pos)
	{
		if (pos < CAPACITY)
		{
			return data_[pos];
		}
		return 0;
	}
	const CharType at(size_t pos)const
	{
		if (pos < CAPACITY)
		{
			return data_[pos];
		}
		return 0;
	}
	CharType* at_ptr(size_t pos)
	{
		if (pos < CAPACITY)
		{
			return begin() + pos;
		}
		return nullptr;
	}
	const CharType* at_ptr(size_t pos)const
	{
		if (pos < CAPACITY)
		{
			return begin() + pos;
		}
		return nullptr;
	}
	void put(size_t pos,CharType c)
	{
		if (pos < length_)
		{
			data_[pos] = c;
		}
	}
	size_t length()const
	{
		return length_;
	}
	void retract()
	{
		if (length_ > 0)
		{
			--length_;
			data_[length_] = 0;
		}
	}
	size_t space()const
	{
		return capacity() - length_;
	}
	bool  empty()const
	{
		return length_ == 0;
	}
	size_t capacity()const
	{
		return CAPACITY - 1;
	}
	template<typename T>
	MyType& append_to_binary(T bin)
	{
		append("0b");
		T bit = (T)1<<(sizeof(T) * sizeof(char) * 8 - 1);
		for (size_t i = 0; i < sizeof(T) * sizeof(char) * 8; ++i)
		{
			if ((bin & bit) > 0)
			{
				append("1");
			}
			else
			{
				append("0");
			}
			bit >>= 1;
		}
		return *this;
	}
	MyType& append(int val,int radix = 10)
	{
		const static int INT_STR_MAX_LEN = 32;
		CharType buffer[INT_STR_MAX_LEN]={};
		int  idx = INT_STR_MAX_LEN;
		int  digit = 0;
		bool sig;
		if (val < 0)
		{
			sig = true;
			val *= -1;
		}else
		{
			sig = false;
		}
		while(val != 0 && idx >= 0)
		{
			digit = val%radix + '0';
			val /= radix;
			if (digit > '9')
			{
				digit += 'A' - '9' - 1;
			}
			buffer[--idx]=digit;
		}
		if (idx == INT_STR_MAX_LEN)
		{
			buffer[--idx] = '0';
		}
		if (sig)
		{
			buffer[--idx] = '-';
		}
		return append(&buffer[idx],INT_STR_MAX_LEN-idx);
	}
	MyType& append(unsigned int val, int radix = 10)
	{
		const static int INT_STR_MAX_LEN = 32;
		CharType buffer[INT_STR_MAX_LEN] = {};
		int  idx = INT_STR_MAX_LEN;
		int  digit = 0;

		while (val != 0 && idx >= 0)
		{
			digit = val%radix + '0';
			val /= radix;
			if (digit > '9')
			{
				digit += 'A' - '9' - 1;
			}
			buffer[--idx] = digit;
		}
		if (idx == INT_STR_MAX_LEN)
		{
			buffer[--idx] = '0';
		}
		return append(&buffer[idx], INT_STR_MAX_LEN - idx);
	}
	MyType& append(long long val,int radix = 10)
	{
		const static int LONG_STR_MAX_LEN = 64;
		CharType buffer[LONG_STR_MAX_LEN]={};
		int  idx = LONG_STR_MAX_LEN;
		int  digit = 0;
		bool sig;
		if (val < 0)
		{
			sig = true;
			val *= -1;
		}else
		{
			sig = false;
		}
		while(val != 0 && idx >= 0)
		{
			digit = val%radix + '0';
			val /= radix;
			if (digit > '9')
			{
				digit += 'A' - '9' - 1;
			}
			buffer[--idx]=digit;
		}
		if (idx == LONG_STR_MAX_LEN)
		{
			buffer[--idx] = '0';
		}
		if (sig)
		{
			buffer[--idx] = '-';
		}
		return append(&buffer[idx],LONG_STR_MAX_LEN-idx);
	}

	MyType& append(const CharType*s)
	{
		return append(s,Traits::length(s));
	}

	MyType& append(const CharType c)
	{
		if (length() < capacity())
		{
			data_[length_++] = c;
			data_[length_] = 0;
		}
		return *this;
	}
	MyType& add(const CharType c)
	{
		if (length() < capacity())
		{
			data_[length_++] = c;
			data_[length_] = 0;
		}
		return *this;
	}
	MyType& append(const CharType*s,size_t len)
	{
		size_t xlen = len;
		if (xlen > capacity() - length())
		{
			xlen = capacity() - length();
		}
		return copyFrom(s,xlen);
	}

	MyType& operator<<(short val)
	{
		return append((int)val);
	}
	MyType& operator<<(unsigned short val)
	{
		return append((int)val);
	}
	MyType& operator<<(int val)
	{
		return append(val);
	}
	MyType& operator<<(unsigned int val)
	{
		return append((long long)val);
	}
	MyType& operator<<(long long val)
	{
		return append(val);
	}
	MyType& operator<<(unsigned long long val)
	{
		return append((long long)val);
	}
	MyType& operator<<(const CharType c)
	{
		return append(c);
	}

	MyType& operator<<(const CharType*s)
	{
		return append(s);
	}

	MyType& operator<<(const static_string_base<CharType,Traits,8>& s)
	{
		return append(s.begin(),s.length());
	}
	
	MyType& operator<<(const static_string_base<CharType,Traits,16>& s)
	{
		return append(s.begin(),s.length());
	}

	MyType& operator<<(const static_string_base<CharType,Traits,32>& s)
	{
		return append(s.begin(),s.length());
	}

	MyType& operator<<(const static_string_base<CharType,Traits,64>& s)
	{
		return append(s.begin(),s.length());
	}

	MyType& operator<<(const static_string_base<CharType,Traits,128>& s)
	{
		return append(s.begin(),s.length());
	}

	MyType& operator<<(const static_string_base<CharType,Traits,256>& s)
	{
		return append(s.begin(),s.length());
	}

	MyType& operator<<(const static_string_base<CharType,Traits,512>& s)
	{
		return append(s.begin(),s.length());
	}

	MyType& operator<<(const static_string_base<CharType,Traits,1024>& s)
	{
		return append(s.begin(),s.length());
	}
	
	MyType& operator<<(const signature_base<CharType,Traits>& s)
	{
		return append(s.begin(),s.length());
	}

	MyType& operator<<(const std::basic_string<CharType, Traits, std::allocator<CharType> >& str)
	{
		return append(str.c_str(),str.length());
	}

	size_t replace(const CharType*s,size_t off,size_t count)
	{
		if (off < capacity())
		{
			if (count >= capacity() - off)
			{
				count = capacity() - off;
			}
			Traits::copy(at_ptr(off),s,count);
			length_ = off + count;
			return count;
		}
		return 0;
	}
	size_t replace(CharType c,size_t off)
	{
		if (off < capacity())
		{
			data_[off] = c;
			return off;
		}
		return 0;
	}
	size_t find_and_replace(CharType find_c,CharType replace_c)
	{
		size_t count = 0;
		for (size_t i = 0; i < length_; ++i)
		{
			if (data_[i] == find_c)
			{
				data_[i] = replace_c;
				++count;
			}
		}
		return count;
	}
	size_t find(const CharType*s,size_t off,size_t count)
	{
		size_t num;
		if (off < this->length_ && count <= (num = this->length_ - off))
		{	// room for match, look for it
			const CharType *_Uptr, *_Vptr;
			for (num -= count - 1, _Vptr = begin() + off;
				(_Uptr = Traits::find(_Vptr, num, *s)) != 0;
				num -= _Uptr - _Vptr + 1, _Vptr = _Uptr + 1)
				if (Traits::compare(_Uptr, s, count) == 0)
					return (_Uptr - begin());	// found a match
		}
		return std::string::npos;
	}
	size_t find(CharType c,size_t off = 0)
	{
		for (size_t i = off; i < length_; i++)
		{
			if (data_[i] == c)
			{
				return i;
			}
		}
		return size_t(-1);
	}
	void clear()
	{
		memset(data_,0,sizeof(data_));
		length_ = 0;
	}
	void reset()
	{
		data_[0] = 0;
		length_  = 0;
	}
	void reset(size_t len)
	{
		if (len > CAPACITY)
		{
			len = CAPACITY;
		}
		length_  = len;
		data_[len] = 0;
	}
	void setsize(size_t len)
	{
		if (len > CAPACITY)
		{
			len = CAPACITY;
		}
		length_  = len;
	}
	inline signature_base<CharType,Traits> to_signature()
	{
		typedef signature_base<CharType,Traits>	signature;
		return signature(begin(),length());
	}
private:
	void init(const CharType*s)
	{
		if (s != NULL)
		{
			if (length_ >= capacity())
			{
				length_ = capacity();
			}
			Traits::copy(begin(),s,length_);
			data_[length_] = 0;
		}
	}
	MyType& copyFrom(const CharType*s,size_t len)
	{
		Traits::copy(end(),s,len);
		length_ += len;
		data_[length_] = 0;
		return *this;
	}
private:
	size_t		length_;
	CharType	data_[CAPACITY];
};

template<typename CharType,typename Traits,size_t CAPACITY> inline
bool operator<(const static_string_base<CharType,Traits,CAPACITY>& left,
				const static_string_base<CharType,Traits,CAPACITY>& right)
{
	return left.compare(right) == -1;
}
template<typename CharType,typename Traits,size_t CAPACITY> inline
	bool operator>(const static_string_base<CharType,Traits,CAPACITY>& left,
	const static_string_base<CharType,Traits,CAPACITY>& right)
{
	return left.compare(right) == 1;
}

template<typename CharType,typename Traits,size_t CAPACITY> inline
	bool operator==(const static_string_base<CharType,Traits,CAPACITY>& left,
	const static_string_base<CharType,Traits,CAPACITY>& right)
{
	return left.compare(right) == 0;
}

template<typename CharType,typename Traits,size_t CAPACITY> inline
	bool operator==(const static_string_base<CharType,Traits,CAPACITY>& left,
	const CharType* right)
{
	return left.compare(right) == 0;
}

template<typename CharType,typename Traits,size_t CAPACITY> inline
	bool operator!=(const static_string_base<CharType,Traits,CAPACITY>& left,
	const static_string_base<CharType,Traits,CAPACITY>& right)
{
	return !(left==right);
}
template<typename sstring_type>
std::string& operator<<(std::string&str,sstring_type&ss)
{
	str.append(ss.begin(),ss.length());
	return str;
}

#define STATIC_STRING_TYPE(t,N) static_string_base<t,std::char_traits<t>,N>
#define STATIC_CHAR_STRING(N)	static_string_base<char,std::char_traits<char>,N>
typedef	STATIC_STRING_TYPE(char,8)				sstring8;
typedef	STATIC_STRING_TYPE(char,16)				sstring16;
typedef	STATIC_STRING_TYPE(char,32)				sstring32;
typedef	STATIC_STRING_TYPE(char,64)				sstring64;
typedef	STATIC_STRING_TYPE(char,128)			sstring128;
typedef	STATIC_STRING_TYPE(char,256)			sstring256;
typedef	STATIC_STRING_TYPE(char,512)			sstring512;
typedef	STATIC_STRING_TYPE(char,1024)			sstring1k;
typedef	STATIC_STRING_TYPE(char,1024*2)			sstring2k;
typedef	STATIC_STRING_TYPE(char,1024*3)			sstring3k;
typedef	STATIC_STRING_TYPE(char,1024*4)			sstring4k;
typedef	STATIC_STRING_TYPE(char,1024*8)			sstring8k;
typedef	STATIC_STRING_TYPE(char,1024*16)		sstring16k;
typedef	STATIC_STRING_TYPE(char,1024*32)		sstring32k;
typedef	STATIC_STRING_TYPE(char,1024*64)		sstring64k;
typedef	STATIC_STRING_TYPE(char,1024*128)		sstring128k;

typedef sstring32								nstring;//namestring

typedef	STATIC_STRING_TYPE(wchar_t, 8)				wsstring8;
typedef	STATIC_STRING_TYPE(wchar_t, 16)				wsstring16;
typedef	STATIC_STRING_TYPE(wchar_t, 32)				wsstring32;
typedef	STATIC_STRING_TYPE(wchar_t, 64)				wsstring64;
typedef	STATIC_STRING_TYPE(wchar_t, 128)			wsstring128;
typedef	STATIC_STRING_TYPE(wchar_t, 256)			wsstring256;
typedef	STATIC_STRING_TYPE(wchar_t, 512)			wsstring512;
typedef	STATIC_STRING_TYPE(wchar_t, 1024)			wsstring1k;
typedef	STATIC_STRING_TYPE(wchar_t, 2048)			wsstring2k;
#endif


