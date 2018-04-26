//sstring utility
#ifndef sutility_hpp_sim_
#define sutility_hpp_sim_

#include "sstring.hpp"
#include "signature.hpp"
#include <string>
#include <cwchar>
#include <cwctype>
#include <locale>
#include <codecvt>
#include <vector>

template<typename StringType,typename CharType,typename Traits,size_t L = 8>
class string_separator
{
public:
	typedef	StringType										string_type;
	typedef signature_base< CharType,Traits >				signature_type;

	string_separator()
		:count_(0)
	{
	}
	string_separator(const CharType* s,size_t len)
		:converter_(s,len),count_(0)
	{
	}
	string_separator(signature_type& s)
		:converter_(s), count_(0)
	{
	}
	void reset_source(const CharType* s, size_t len)
	{
		converter_.reset(s, len);
		count_ = 0;
	}
	void reset_source_string(string_type& s)
	{
		converter_.reset(s.data(), s.length());
		count_ = 0;
	}
	void reset_source(const signature_type& s)
	{
		converter_.reset(s.begin(), s.length());
		count_ = 0;
	}
	size_t split(CharType deli)
	{
		const CharType* b = converter_.begin();
		const CharType* e = converter_.end();
		signature_type* sig = &current();
		sig->reset(b,0);
		while (b != e)
		{
			if (*b == deli)
			{
				if(next())//++count_;
				{
					sig = &current();
					sig->reset(++b,0);
					continue;
				}else
				{
					return count_;
				}

			}else
			{
				sig->grow();
			}

			++b;
		}
		return count_;
	}
	signature_type& get(size_t index)
	{
		ASSERT(index <= count_);
		return valt_[index];
	}
	signature_type& operator[](size_t index)
	{
		if(index > count_) throw "out of range.";
		return valt_[index];
	}
	/*
	delimiter count
	*/
	size_t count()const
	{
		return count_;
	}
private:
	signature_type & current()
	{
		ASSERT(count_ < L);
		return valt_[count_];
	}
	bool next()
	{
		if ((count_ + 1) < L)
		{
			++count_;
			return true;
		}else
		{
			return false;
		}
	}
private:
	size_t				count_;
	signature_type		converter_;
	signature_type		valt_[L];
};

#define STATIC_STR_SPARATOR_TYPE(s,t,L) string_separator<s,t,std::char_traits<t>,L>

typedef	STATIC_STR_SPARATOR_TYPE(std::string,char,8)			sseparator_8;
typedef	STATIC_STR_SPARATOR_TYPE(signature,char,8)				sigseparator_8;
typedef	STATIC_STR_SPARATOR_TYPE(sstring64,char,8)				ssseparator64_8;
typedef	STATIC_STR_SPARATOR_TYPE(sstring256,char,8)				ssseparator256_8;
typedef	STATIC_STR_SPARATOR_TYPE(sstring256,char,16)			ssseparator256_16;


template<typename StringType, typename CharType, typename Traits>
class string_split_base
{
public:
	typedef	StringType										string_type;
	typedef signature_base< CharType, Traits >				signature_type;

	string_split_base()
		:cur_ptr_(nullptr)
	{
	}
	string_split_base(const CharType* s, size_t len)
		:converter_(s, len), line_(s, 0), cur_ptr_(s)
	{
	}
	string_split_base(const string_type& s)
		:converter_(s), line_(s.data(), 0), cur_ptr_(s.data())
	{
	}
	string_split_base(signature_type& s)
		:converter_(s), line_(s.begin(), 0), cur_ptr_(s.begin())
	{
	}
	void reset_source(const CharType* s, size_t len)
	{
		converter_.reset(s, len);
		line_.reset(s, 0);
		cur_ptr_ = s;
	}
	void reset_source(string_type& s)
	{
		converter_.reset(s.data(), s.length());
		line_.reset(s.data(), 0);
		cur_ptr_ = s.data();
	}
	void reset_source(signature_type& s)
	{
		converter_.reset(s.begin(), s.length());
		line_.reset(s.begin(), 0);
		cur_ptr_ = s.begin();
	}

	void reset_source()
	{
		cur_ptr_ = converter_.begin();
		line_.reset(converter_.begin());
	}
	signature_type& get_next(CharType deli)
	{
		const CharType* e = converter_.end();
		line_.reset(cur_ptr_);
		while (cur_ptr_ != e)
		{
			if (*cur_ptr_ == deli)
			{
				++cur_ptr_;//skip deli char.
				break;
			}
			else
			{
				line_.grow();
			}
			++cur_ptr_;
		}
		return line_;
	}
	signature_type& get_next_real(CharType deli)
	{
		const CharType* e = converter_.end();
		line_.reset(cur_ptr_);
		while (cur_ptr_ != e)
		{
			if (*cur_ptr_ == deli)
			{
				++cur_ptr_;//skip deli char.
				if(!line_.empty())
				break;
				else
				{
					line_.reset(cur_ptr_);
				}
			}
			else
			{
				line_.grow();
			}
			++cur_ptr_;
		}
		return line_;
	}
	template<typename Handler>
	signature_type& check_next(CharType deli,Handler& handle)
	{
		while (!handle(get_next(deli)))
		{			
		};
		return line_;
	}
private:
	const CharType*		cur_ptr_;
	signature_type		converter_;
	signature_type		line_;
};

typedef string_split_base<std::string, char, std::char_traits<char>>			string_split;
typedef string_split_base<std::wstring, wchar_t, std::char_traits<wchar_t>>		wstring_split;


struct char_trait_utility
{
	static inline int compare(const char* src1, const char* src2)
	{
		return strcmp(src1, src2);
	}
	static inline size_t length(const char* src)
	{
		return strlen(src);
	}
};

struct wchar_trait_utility
{
	static int compare(const wchar_t* src1, const wchar_t* src2)
	{
		return wcscmp(src1, src2);
	}
	static inline size_t length(const wchar_t* src)
	{
		return wcslen(src);
	}
};
template<typename CharT, typename CharU,int M>
struct str_array_index_utility
{
	typedef CharT MChar;
	typedef const MChar* StrArray[M];
	StrArray& data;
	str_array_index_utility(StrArray& arr)
		:data(arr)
	{}
	int GetIndex(const MChar* str)
	{
		for (int i = 0; i < M; i++)
		{
			if (CharU::compare(str, data[i]) == 0)
			{
				return i;
			}
		}
		return M;
	}
	const MChar* GetString(int index)
	{
		if (index >= 0 && index < M)
		{
			return data[index];
		}
		return nullptr;
	}
};

#define	STR_ARRAY_INDEX_UT(N)  str_array_index_utility<char,char_trait_utility,N>
#define	WSTR_ARRAY_INDEX_UT(N) str_array_index_utility<wchar_t,wchar_trait_utility,N>
//sample typedef
typedef STR_ARRAY_INDEX_UT(5)	csarray5index_u;
typedef WSTR_ARRAY_INDEX_UT(5)	wsarray5index_u;


template<typename StrT>
inline size_t find_sub_string(const StrT& str, size_t begin_pos,const StrT& sub)
{
	size_t eq_cnt = 0;
	for (size_t i = begin_pos; i < str.length(); ++i)
	{
		eq_cnt = 0;
		for (size_t j = 0; j < str.length(); ++j)
		{
			if (str[i + j] == sub[j])
			{
				++eq_cnt;
				if (eq_cnt == sub.length())
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
	return std::string::npos;
}

template<typename StrT>
inline size_t find_sub_string(const StrT& str, size_t begin_pos,size_t end_pos, const StrT& sub)
{
	size_t eq_cnt = 0;
	if (end_pos > str.length() ||
		begin_pos > end_pos)
	{
		ASSERT(0);
		return std::string::npos;
	}
	if (end_pos - begin_pos < sub.length())
	{
		return std::string::npos;
	}
	for (size_t i = begin_pos; i < end_pos; ++i)
	{
		eq_cnt = 0;
		for (size_t j = 0; j < sub.length(); ++j)
		{
			if (str[i + j] == sub[j])
			{
				++eq_cnt;
				if (eq_cnt == sub.length())
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
	return std::string::npos;
}

inline size_t find_char_in_string(const std::string & str, size_t begin_pos, char sub)
{
	for (size_t i = begin_pos; i < str.length(); ++i)
	{
		if (str[i] == sub)
		{
			return i;
		}
	}
	return -1;
}
inline size_t find_char_in_string(const std::wstring & str, size_t begin_pos, wchar_t sub)
{
	for (size_t i = begin_pos; i < str.length(); ++i)
	{
		if (str[i] == sub)
		{
			return i;
		}
	}
	return -1;
}
template<typename CharType>
inline size_t compare_string_equal_length(
	const std::basic_string<CharType, std::char_traits<CharType>, std::allocator<CharType> >& str_a,
	const std::basic_string<CharType, std::char_traits<CharType>, std::allocator<CharType> >& str_b)
{
	typedef std::basic_string<CharType, std::char_traits<CharType>, std::allocator<CharType> > this_string;
	size_t len = 0;
	size_t max_len = str_a.length();
	if (str_a.length() < str_b.length())
	{
		max_len = str_b.length();
	}
	for (size_t i = 0; i < max_len; i++)
	{
		if (str_a[i] == str_b[i])
		{
			++len;
		}
		else
		{
			break;
		}
	}
	return len;
}
template<typename CharType>
inline bool is_string_has_num(const std::basic_string<CharType, std::char_traits<CharType>, const std::allocator<CharType> >& str)
{
	for each (CharType ch in str)
	{
		if (isdigit(ch))
		{
			return true;
		}
	}
	return false;
}

inline std::string gbk_to_utf8(std::string const &strGBK)
{
	std::vector<wchar_t> buff(strGBK.size());
#ifdef _MSC_VER
	std::locale loc("chs");
#else
	std::locale loc("zh_CN.GB18030");
#endif
	wchar_t* pwszNext = nullptr;
	const char* pszNext = nullptr;
	mbstate_t state = {};
	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
		(loc).in(state,
			strGBK.data(), strGBK.data() + strGBK.size(), pszNext,
			buff.data(), buff.data() + buff.size(), pwszNext);

	if (std::codecvt_base::ok == res)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
		return cutf8.to_bytes(std::wstring(buff.data(), pwszNext));
	}

	return "";

}

inline std::string utf8_to_gbk(std::string const &strUtf8)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
	std::wstring wTemp = cutf8.from_bytes(strUtf8);
#ifdef _MSC_VER
	std::locale loc("chs");
#else
	std::locale loc("zh_CN.GB18030");
#endif
	const wchar_t* pwszNext = nullptr;
	char* pszNext = nullptr;
	mbstate_t state = {};

	std::vector<char> buff(wTemp.size() * 2);
	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
		(loc).out(state,
			wTemp.data(), wTemp.data() + wTemp.size(), pwszNext,
			buff.data(), buff.data() + buff.size(), pszNext);

	if (std::codecvt_base::ok == res)
	{
		return std::string(buff.data(), pszNext);
	}
	return "";
}
inline void StrToHexStr(const std::string& strSource, std::string& strDest)
{
	unsigned char* ch_begin = (unsigned char*)strSource.data();
	unsigned char* ch_end = ch_begin + strSource.length() * sizeof(char);
	const char HEX_NUM_AR[] = "0123456789abcdef";
	char buf[32] = {};
	size_t len = 0;
	while (ch_begin != ch_end)
	{
		unsigned char ch = *ch_begin;
		ch &= 0xF0;
		ch >>= 4;
		buf[len++] = HEX_NUM_AR[(ch & 0xF)];
		buf[len++] = HEX_NUM_AR[(*ch_begin & 0xF)];
		++ch_begin;
	}
	strDest.append(buf, len);
}
inline void StrToHexStr(const std::wstring& strSource, std::string& strDest)
{
	unsigned char* ch_begin = (unsigned char*)strSource.data();
	unsigned char* ch_end = ch_begin + strSource.length() * sizeof(wchar_t);
	const char HEX_NUM_AR[] = "0123456789abcdef";
	char buf[32] = {};
	size_t len = 0;
	while (ch_begin != ch_end)
	{
		unsigned char ch = *ch_begin;
		ch &= 0xF0;
		ch >>= 4;
		buf[len++] = HEX_NUM_AR[(ch & 0xF)];
		buf[len++] = HEX_NUM_AR[(*ch_begin & 0xF)];
		++ch_begin;
	}
	strDest.append(buf, len);
}
namespace StdStrCvt
{
//UTF-8<->Unicode×ª»»Æ÷
static std::wstring_convert<std::codecvt_utf8<wchar_t>>	convert_utf8_unicode;
//GBK<->Unicode×ª»»Æ÷
static std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>
convert_gbk_unicode(new std::codecvt<wchar_t, char, std::mbstate_t>("CHS"));

}
#endif // sutility_h__


