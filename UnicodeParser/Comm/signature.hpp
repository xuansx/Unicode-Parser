#ifndef signature_h_sim
#define signature_h_sim

#include <string>
#include <stdio.h>

#ifndef ASSERT
#if defined _DEBUG
#include <assert.h>
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif
#endif

template<typename CharType,typename Traits>
class signature_base
{
public:
	typedef		Traits											traits_type;
	typedef		CharType										char_type;
	typedef		std::basic_string<CharType, Traits, std::allocator<CharType>>	string_type;
	typedef		signature_base<CharType,Traits>	MyType;
	const		size_t npos = string_type::npos;
	signature_base()
		:str_(NULL),length_(0)
	{}
	signature_base(const CharType* s)
		:str_(s),length_(0)
	{
		if (s)
		{
			length_ = Traits::length(s);
		}
	}
	signature_base(const CharType* s,size_t count)
		:str_(s),length_(count)
	{}
	signature_base(const string_type& str)
		:str_(str.data()), length_(str.length())
	{}
	signature_base(const MyType& other)
		:str_(other.str_),length_(other.length_)
	{}
	operator bool()
	{
		if (str_ != NULL)
		{
			return true;
		}
		return false;
	}
	void operator=(const MyType& other)
	{
		str_ = other.str_;
		length_ = other.length_;
	}
	bool operator==(const MyType& other)
	{
		return equal(other);
	}
	void swap(MyType& other)
	{
		CharType* s	= str_;
		size_t count= length_;
		str_ = other.str_;
		length_ = other.length_;
		other.str_ = s;
		other.length_ = count;
	}
	void reset(const CharType* s = NULL,size_t count = 0)
	{
		str_ = s;
		length_ = count;
	}
	void reset(const MyType& other)
	{
		str_ = other.str_;
		length_ = other.length_;
	}
	MyType sub(size_t begin_pos, size_t len)
	{
		if (begin_pos >= length())
		{
			return MyType();
		}
		if (len > length() - begin_pos)
		{
			len = length() - begin_pos;
		}
		return MyType(at(begin_pos), len);
	}
	MyType sub_tail(size_t begin_pos)
	{
		if (begin_pos >= length())
		{
			return MyType();
		}
		return MyType(at(begin_pos), len);
	}
	const CharType* begin()const
	{
		return str_;
	}
	const CharType* end()const
	{
		return begin()+length_;
	}
	const CharType* at(size_t pos)const
	{
		if (pos < length_)
		{
			return begin() + pos;
		}
		return NULL;
	}
	bool tail_is_alpha()const
	{
		return (isalpha(str_[length_])!=0)||(str_[length_] == CharType('_'));
	}
	bool tail_is_digit()const
	{
		return isdigit(str_[length_])!=0;
	}
	bool tail_is_xdigit()const
	{
		return isxdigit(str_[length_])!=0;
	}
	bool is_xdigit_string()const
	{
		const CharType* b = begin();
		for (; b != end(); ++b)
		{
			if (isxdigit(*b) == 0)
			{
				return false;
			}
		}
		return true;
	}
	bool tail_is(CharType c)const
	{
		return str_[length_] == c;
	}
	size_t find_first(CharType c, size_t begin_pos = 0)const
	{
		if (length_ <= begin_pos)
		{
			return npos;
		}
		const CharType* str = str_ + begin_pos;
		while (str != end())
		{
			if (*str++ == c)
			{
				return str - str_;
			}
		}
		return npos;
	}
	size_t find_first(const string_type& substr, size_t begin_pos = 0)const
	{
		return find_first(substr.data(), substr.length(), begin_pos);
	}
	size_t find_first(const MyType& substr, size_t begin_pos = 0)const
	{
		return find_first(substr.begin(), substr.length(), begin_pos);
	}
	size_t find_first(const CharType* substr, size_t subStrLen, size_t begin_pos = 0)const
	{
		if (length_ < (subStrLen + begin_pos))
		{
			return npos;
		}
		size_t b_pos = begin_pos;
		int cmpr = Traits::compare(str_ + b_pos, substr, subStrLen);
		while (cmpr != 0 &&
			(b_pos < length_ - subStrLen))
		{
			++b_pos;
			cmpr = Traits::compare(str_ + b_pos, substr, subStrLen);
		}
		if (cmpr == 0)
			return b_pos;
		else
			return npos;
	}
	size_t find_tail(const string_type& substr)const
	{
		if (length_ < substr.length())
		{
			return npos;
		}
		size_t b_pos = length_ - substr.length();
		int cmpr = Traits::compare(str_+ b_pos, substr.c_str(), substr.length());
		while (b_pos > 0 && cmpr != 0)
		{
			--b_pos;
			cmpr = Traits::compare(str_ + b_pos, substr.c_str(), substr.length());
		}
		if (cmpr == 0)
			return b_pos;
		else
			return npos;
	}
	void trace()
	{
		//static const char* s_trace = " \t\n\r\v\f";
		if (!str_ || length_ == 0)
		{
			return;
		}
		while ((*str_ <= 32) && (*str_ > 0) && length_ > 0)
		{
			++str_;
			--length_;
		}
		if (length_ == 0)
		{
			return;
		}
		const CharType* str = str_+ (length_ - 1);
		while ((*str <= 32) && (*str > 0) && length_ > 0)
		{
			--str;
			--length_;
		}
	}
	void trace_tail()
	{
		//static const char* s_trace = " \t\n\r\v\f";
		if (!str_ || length_ == 0)
		{
			return;
		}
		const CharType* str = str_ + (length_ - 1);
		while ((*str <= 32) && (*str > 0) && length_ > 0)
		{
			--str;
			--length_;
		}
	}
	void move_begin_to_end()
	{
		ASSERT(str_ != NULL);
		str_ = &str_[length_];
		length_ = 0;
	}
	
	void move_begin_to_tail(size_t cnt)
	{
		ASSERT(str_ != NULL);
		if (cnt >= length_)
		{
			length_ = 0;
		}
		else
		{
			length_ -= cnt;
		}
		str_ = &str_[cnt];		
	}
	size_t length()const
	{
		return length_;
	}
	bool empty()const
	{
		if (str_ == nullptr)
		{
			return true;
		}
		if (length_ == 0)
		{
			return true;
		}
		return false;
	}
	int compare(const MyType& other)const
	{
		return compare(other.begin(),other.length());
	}
	int compare(const CharType* s)const
	{
		ASSERT( s != NULL );
		return compare(s,Traits::length(s));
	}
	int compare(const CharType* s,size_t count)const
	{
		ASSERT(s != NULL);
		size_t len = length();
		if (len > count)
		{
			len = count;
		}
		int cmpr =  Traits::compare(str_, s, len);
		if ((cmpr == 0) && (count != length()))
		{
			if (length() < count)
			{
				return -1;
			}else
			{
				return 1;
			}
		}
		return cmpr;
	}
	bool equal(const CharType* s)const
	{
		ASSERT(s != NULL);
		return equal(s,Traits::length(s));
	}
	bool equal(const MyType& other)const
	{
		return equal(other.begin(),other.length());
	}
	bool equal(const string_type& str)const
	{
		return equal(str.c_str(), str.length());
	}
	bool equal(const CharType* s,size_t count)const
	{
		ASSERT(s != NULL);
		if (length_ != count)
		{
			return false;
		}
		if (s == str_)
		{
			return true;
		}
		int res = Traits::compare(str_, s, length_);
		return res == 0;
	}
	size_t contain(const CharType* s,size_t _length,size_t start_pos,size_t end_pos)const
	{
		if (length() < _length)
		{
			return npos;
		}
		size_t eq_cnt = 0;
		for (size_t i = start_pos; i < end_pos; ++i)
		{
			eq_cnt = 0;
			for (size_t j = 0; j < _length; ++j)
			{
				if (str_[i + j] == s[j])
				{
					++eq_cnt;
					if (eq_cnt == _length)
					{
						return i;
					}
				}
				else
				{
					break;
				}
			}
		}
		return npos;
	}
	size_t contain(const CharType* s, size_t _length)const
	{
		return contain(s, _length, 0, length());
	}
	string_type to_string()
	{
		return string_type(str_,length_);
	}
	int	 to_int(int base = 0)
	{
		return to_integer<int>(str_, base);
	}
	int	 to_int(size_t start_pos,int base = 0)
	{
		return to_integer<int>((str_ + start_pos), base);
	}
	unsigned int to_uint(int base = 0)
	{
		return to_integer<unsigned int>(str_, base);
	}
	long long to_long(int base = 0)
	{
		return to_integer<long long>(str_, base);
	}
	unsigned long long to_ulong(int base = 0)
	{
		return to_integer<unsigned long long>(str_,base);
	}
	float to_float()
	{
		char buf[32] = {};
		strncpy(buf, str_,sizeof(buf));
		return atof(buf);
	}
	bool to_bool()
	{
		if (equal("true",4))
		{
			return true;
		}
		return false;
	}
	template<typename CharT>
	bool to_bool()
	{
		if (equal(L"true", 4))
		{
			return true;
		}
		return false;
	}
	unsigned char binary_to_uint8()
	{
		return binary_to_integer<unsigned char>();
	}
	unsigned short binary_to_uint16()
	{
		return binary_to_integer<unsigned short>();
	}
	unsigned long binary_to_uint32()
	{
		return binary_to_integer<unsigned long>();
	}
	unsigned long long binary_to_uint64()
	{
		return binary_to_integer<unsigned long long>();
	}
	size_t grow(size_t num = 1)
	{
		return length_+= num;
	}
	size_t hash_code()
	{
		unsigned char *_First = (unsigned char* )begin();
		size_t _Count = length()*sizeof(CharType);
		// FNV-1a hash function for bytes in [_First, _First + _Count)

		static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
		const size_t _FNV_offset_basis = 14695981039346656037ULL;
		const size_t _FNV_prime = 1099511628211ULL;

		size_t _Val = _FNV_offset_basis;
		for (size_t _Next = 0; _Next < _Count; ++_Next)
		{	// fold in another byte
			_Val ^= (size_t)_First[_Next];
			_Val *= _FNV_prime;
		}
		return (_Val);
	}
private:
#pragma warning(disable:4146)
	template<typename NumberType>
	NumberType	 to_integer(const CharType* p,int base)
	{
// 		static int  HexUpDiff = 'A' - '9';
// 		static int  HexLowerDiff = 'A' - '9';
		//const CharType* p	= str_;
		size_t len			= length_;
		NumberType	r	= 0;
		NumberType	tp	= 0;
		bool		sig = true;
		if (len <= 0)
		{
			return 0;
		}
		if (base == 0)
		{
			base = 10;
			if (len >= 2)
			{
				if (p[0] == '0' && p[1] == 'x')
				{
					base = 16;
					++p;
					++p;
				}
			}
		}
		if (*p == '+')
		{
			++p;
			--len;
		}
		if (*p == '-')
		{
			sig = false;
			++p;
			--len;
		}
		while(len > 0)
		{
			if (!(*p))
			{
				return r;
			}
			len--;
			if (*p >= '0' && *p <= '9')
			{
				tp =*p++ - '0';
				if (tp == 0 && r == 0)
				{
					continue;
				}
			}else
			{
				if (base == 16)
				{
					if (*p >= 'a' && *p <= 'f')
					{
						tp = *p++ - 'a' +10;
					}
					else if (*p >= 'A' && *p <= 'F')
					{
						tp = *p++ - 'A' +10;
					}else
					{
						break;
					}
				}else
				{
					break;
				}
			}

			if (r == 0)
			{
				r = tp;
				continue;
			}

			r = r*base + tp;
		}

		if (!sig)
		{
			r = -r;
		}

		return r;
	}
	template<typename NumberType>
	NumberType	 binary_to_integer()
	{
		const CharType* p = str_;
		NumberType res = 0;
		if (length_ < 2)
		{
			return res;
		}
		if (p[0] == '0' && p[1] == 'b')
		{
			++p;
			++p;
		}
		while (p != end())
		{
			if (*p == '0')
			{
				res <<= 1;
			}else if (*p == '1')
			{
				res <<= 1;
				res += 1;
			}else
			{
				break;
			}
			++p;
		}
		return res;
	}
#pragma warning(default:4146)
private:
	const CharType*	str_;
	size_t		length_;
};

//template<typename CharType, typename Traits>
//size_t signature_base<CharType, Traits>::npos = std::basic_string<CharType, Traits, std::allocator<CharType>>::npos;

template<typename CharType,typename Traits> inline
	bool operator<(const signature_base<CharType,Traits>& left,
	const signature_base<CharType,Traits>& right)
{
	return left.compare(right) < 0;
}

template<typename CharType,typename Traits> inline
	bool operator>(const signature_base<CharType,Traits>& left,
	const signature_base<CharType,Traits>& right)
{
	return left.compare(right) > 0;
}

template<typename CharType,typename Traits> inline
	bool operator==(const signature_base<CharType,Traits>& left,
	const signature_base<CharType,Traits>& right)
{
	return left.equal(right);
}

template<typename CharType,typename Traits> inline
	bool operator!=(const signature_base<CharType,Traits>& left,
	const signature_base<CharType,Traits>& right)
{
	return !left.equal(right);
}

typedef signature_base< char,std::char_traits<char> >	signature;
typedef signature_base< wchar_t, std::char_traits<wchar_t>>	wsignature;

#endif

