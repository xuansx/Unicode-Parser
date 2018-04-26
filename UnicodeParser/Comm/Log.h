#ifndef log_h_sam_
#define log_h_sam_

#include "sarray.h"
#include "sstring.hpp"
#include "SharedMemory.h"
#include "Singleton.h"
#include <iostream>
#include <mutex>


enum eLogLevel
{
	LOG_LVL_NONE,
	LOG_LVL_INFO,
	LOG_LVL_TRACE,
	LOG_LVL_WARNING,
	LOG_LVL_FATAL,
	LOG_LVL_DEBUG,
};

static const size_t LOG_FILE_MAX_LEN	= 1024*1024*1;
static const size_t LOG_SAFE_LEFT_LEN	= 1024;
static const size_t MAX_LOG_SINK		= 8;

template<typename CharType,typename Traits,size_t LOG_SAFE_LEFT_LEN = 8192>
struct log_string_base
{
	typedef log_string_base<CharType,Traits,LOG_SAFE_LEFT_LEN>	MyType;
	log_string_base()
		:level_(0),length_(0),last_len_(0),content_(nullptr)
	{}
	CharType level()const
	{
		return level_;
	}
	void level(CharType lvl)
	{
		level_ = lvl;
	}
	void reset(CharType* ct,size_t cap)
	{
		content_ = ct;
		capacity_ = cap;
		length_ = 0;
	}
	bool next(size_t len = LOG_SAFE_LEFT_LEN)
	{
		if (length_ + len > capacity_)
		{
			return false;
		}
		last_len_ = length_;
		return true;
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
	MyType& append(const CharType* info,size_t len)
	{
		if (len + length_ >= capacity_)
		{
			len = capacity_ - length_;
		}
		memcpy(current_content(),info,len);
		length_ += len;
		return *this;
	}
	MyType& append(const CharType*s)
	{
		return append(s,Traits::length(s));
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
	MyType& operator<<(const CharType*s)
	{
		return append(s);
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
	int printf(const char* format,...)
	{
		int		res = 0;
		va_list args;
		va_start(args,format);
#ifdef WIN32
		res = vsprintf_s(current_content(),capacity()-length(),format,args);
#else
		vsprintf(current_content(),format,args);
#endif
		va_end(args);
		if (res >=capacity())
		{
			res = int(capacity() - 1);
		}
		length_    = res + length_;
		return res;
	}
	int vprintf(const char* format,va_list args)
	{
		int		res = 0;
#ifdef WIN32
		res = vsprintf_s(current_content(),capacity()-length(),format,args);
#else
		vsprintf(current_content(),format,args);
#endif
		if (res >=capacity())
		{
			res = int(capacity() - 1);
		}
		length_    = res + length_;
		return res;
	}
	bool is_screen()const
	{
		switch (level())
		{
		case LOG_LVL_TRACE:
		case LOG_LVL_WARNING:
		case LOG_LVL_FATAL:
		case LOG_LVL_DEBUG:
			return true;
		}
		return false;
	}
	size_t capacity()const
	{
		return capacity_;
	}
	size_t length()const
	{
		return length_;
	}
	size_t last_length()const
	{
		return last_len_;
	}
	size_t diff_length()const
	{
		if (length_ > last_len_)
		{
			return length_ - last_len_;
		}
		return 0;
	}
	char*  current_content()
	{
		if (content_ == nullptr)
		{
			return nullptr;
		}
		return content_+length_;
	}
	char* last_content()
	{
		return content_+last_len_;
	}
	size_t length_;
	size_t	last_len_;
	size_t capacity_;
	char*  content_;
	char   level_;
};

typedef log_string_base<char,std::char_traits<char>,8192>	logstring;

struct LogSink 
{
	LogSink(int32 sinkid,int32 svrID,const char* profix_name = nullptr,const char* proj_name= nullptr);
	~LogSink();
	logstring& Log(char lvl = 0)
	{
		mutex_.lock();
		if (!logstr.next())
		{
			recreate();
		}
		if (lvl != 0)
		{
			updateTimestamp(GetSysTime(),false);
			timestampBuf.replace(lvl+'0',1);
			logstr.level(lvl);
			logstr<<timestampBuf;
		}
		return logstr;
	}
	void Flush(char sink = 0)
	{
		logstr.append("\n",1);
		if (logstr.is_screen())
		{
			std::cout.write(logstr.last_content(),logstr.diff_length());
		}
		mutex_.unlock();
	}
private:
	void recreate();
	void updateTimestamp(time_tick tick,bool bFile = true);
private:
	int32			serverID;
	time_tick		lastTick;
	sstring64		timestampBuf;
	logstring		logstr;
	SharedMemory	memory;
	std::mutex		mutex_;
	uint8			sinkID;
};


struct LogManager:public SingletonStaticT<LogManager>
{
	typedef static_array<LogSink*,uint8,MAX_LOG_SINK>	SinkArray;
	LogManager()
		:serverID(0)
	{
		sinks.make_full();
	}
	~LogManager()
	{
	}
	void Startup(int32 svrID = 0);
	void Shutdown();
	LogSink* CreateSink(uint8 sinkID,const char* prefix_name = nullptr,const char* proj_name= nullptr );
	LogSink* GetSink(uint8 sinkID)
	{
		if (sinks[sinkID] == nullptr)
		{
			CreateSink(sinkID);
		}
		return sinks[sinkID];
	}
	logstring& Log(char lvl,uint8 sink = 0)
	{
		return GetSink(sink)->Log(lvl);
	}
	void Flush(char sink = 0)
	{
		return sinks[sink]->Flush();
	}
private:
	int32			serverID;
	SinkArray		sinks;
};


#define LOG(x)			{logstring&ls =LogManager::Instance().Log(LOG_LVL_NONE);	ls<<"["<<x<<"]";LogManager::Instance().Flush();}
#define LOG_INFO(x)		{logstring&ls =LogManager::Instance().Log(LOG_LVL_INFO);	ls<<"["<<x<<"]";LogManager::Instance().Flush();}
#define LOG_TRACE(x)	{logstring&ls =LogManager::Instance().Log(LOG_LVL_TRACE);	ls<<"["<<x<<"]";LogManager::Instance().Flush();}
#define LOG_WARNING(x)  {logstring&ls =LogManager::Instance().Log(LOG_LVL_WARNING);	ls<<"["<<x<<"]";LogManager::Instance().Flush();}
#define LOG_FATAL(x)	{logstring&ls =LogManager::Instance().Log(LOG_LVL_FATAL);	ls<<"["<<x<<"]";LogManager::Instance().Flush();}

#define XLOG(s,x)			{logstring&ls =LogManager::Instance().Log(LOG_LVL_NONE,s);	ls<<"["<<x<<"]";LogManager::Instance().Flush(s);}
#define XLOG_INFO(s,x)		{logstring&ls =LogManager::Instance().Log(LOG_LVL_INFO,s);	ls<<"["<<x<<"]";LogManager::Instance().Flush(s);}
#define XLOG_TRACE(s,x)	{logstring&ls =LogManager::Instance().Log(LOG_LVL_TRACE,s);	ls<<"["<<x<<"]";LogManager::Instance().Flush(s);}
#define XLOG_WARNING(s,x)  {logstring&ls =LogManager::Instance().Log(LOG_LVL_WARNING,s);	ls<<"["<<x<<"]";LogManager::Instance().Flush(s);}
#define XLOG_FATAL(s,x)	{logstring&ls =LogManager::Instance().Log(LOG_LVL_FATAL,s);	ls<<"["<<x<<"]";LogManager::Instance().Flush(s);}
#ifdef _DEBUG
#define LOG_DEBUG(x)	{logstring&ls =LogManager::Instance().Log(LOG_LVL_DEBUG);	ls<<"["<<__FILE__<<':'<<__FUNCTION__<<":"<<__LINE__<<">>"<<x<<"]";LogManager::Instance().Flush();}
#define LOG_DEBUGF(x,...)	{logstring&ls =LogManager::Instance().Log(LOG_LVL_DEBUG);	ls<<"["<<__FILE__<<':'<<__FUNCTION__<<":"<<__LINE__;ls.printf(x,__VA_ARGS__);ls.append("]",1);LogManager::Instance().Flush();}
#else
#define LOG_DEBUG(x) LOG_WARNING(x)
#endif

#endif // xlog_h__
