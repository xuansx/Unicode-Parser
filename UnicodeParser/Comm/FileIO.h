#pragma once
#ifndef FileIO_H_
#define FileIO_H_
#include <string>
#include "SysTraits.h"


void		MakeDir(const char* path);
int			DelFile(const char* filename);

struct FileIOState
{
public:
	enum eState {
		St_OK,
		St_EOF,
		St_IOError,
		St_Unknow,
	};
	FileIOState()
		:state_(St_Unknow)
	{
	}
	bool Good()const
	{
		return state_ == St_OK;
	}
	eState GetState()const
	{
		return state_;
	}
protected:
	inline void SetState(eState st)
	{
		state_ = st;
	}
protected:
	eState	state_;
};


struct WFileState {
	enum eCodeType
	{
		CT_NONE,
		CT_UTF8,
		CT_UNICODE,
		//CT_GBK,
	};
};




class XFileBase:public FileIOState {
public:
	enum eFileOp {
		Op_ReadOnly,
		Op_ReadEx,
		Op_ReadBinaryEx,
		Op_ReadTextEx,
		Op_WriteOnly,
		Op_WriteEx,
		Op_Append,
		Op_AppendEx,
		Op_WriteBinary,
		Op_WriteBinaryEx,
		Op_WriteTextEx,
		Op_AppendTextEx,
		Op_AppendBinaryEx,
	};
	enum {
		Op_Count = Op_AppendBinaryEx + 1
	};
public:
	XFileBase()
		:handle_(0)
	{}
	~XFileBase()
	{
		closeFile();
	}
	size_t Size();
	bool Write(const void* buffer, size_t length);
	bool Read(void* buffer, size_t& length);
	void ResetSeek();
	void Close()
	{
		closeFile();
	}
protected:
	void closeFile();
	inline FILE* handle()
	{
		return handle_;
	}
	inline void handle(FILE* fileH)
	{
		handle_ = fileH;
	}
	inline FILE** handlePtr()
	{
		return &handle_;
	}
private:
	FILE*			handle_;
};

struct XFile:public XFileBase {
public:
	typedef char		char_type;
	static const char* FileOpStr[Op_Count];
public:
	bool Open(const char* filename, eFileOp op);
};
struct XWFile :public XFileBase,public WFileState {
public:
	typedef wchar_t		char_type;
	static const wchar_t* WFileOpStr[Op_Count];
public:
	bool Open(const wchar_t* filename, eFileOp op,eCodeType code = CT_NONE);
};

template<typename BasicFile>
class BasicBufferFile :public BasicFile
{
public:
	typedef BasicFile	BasicFileType;
	typedef typename BasicFileType::char_type	char_type;
public:
	enum {
		kBufferFenceSize = 4//add extro buffer for string \0;
	};
	BasicBufferFile()
		:buffer_(nullptr),length_(0)
	{
	}
	~BasicBufferFile()
	{
		releaseBuffer();
	}
	bool ReadToBuffer(const char_type* filename)
	{
		if (!Open(filename, BasicFileType::Op_ReadOnly))
		{
			return false;
		}
		releaseBuffer();
		length_ = Size();
		allocateBuffer(length_+ kBufferFenceSize);
		if (!Read(buffer_, length_))
		{
			return false;
		}
		closeFile();
		return true;
	}
	void WriteToBuffer(const void* source, size_t length)
	{
		if (length_ < length)
		{
			releaseBuffer();
			length_ = length;
			allocateBuffer();
		}
		else
		{
			length_ = length;
		}
		memcpy(buffer_, source, length);
	}
	bool SaveBuffer(const char_type* filename)
	{
		if (!Open(filename, BasicFileType::Op_WriteOnly))
		{
			return false;
		}
		if (!Write(buffer_, length_))
		{
			return false;
		}
		closeFile();
		return true;
	}
	const char* GetBuffer()const
	{
		return buffer_;
	}
	const char* GetBuffer(size_t index)const
	{
		if (index < length_)
		{
			return nullptr;
		}
		return buffer_+index;
	}
	size_t GetLength()
	{
		return length_;
	}
private:
	void releaseBuffer()
	{
		if (buffer_)
		{
			free(buffer_);
			buffer_ = nullptr;
		}
	}
	void* allocateBuffer(size_t length)
	{
		buffer_ = (char*)malloc(length);
		memset(&buffer_[length_], 0, kBufferFenceSize);
		return buffer_;
	}
private:
	char*	buffer_;
	size_t	length_;
};

typedef BasicBufferFile<XFile>		BufferFile;
typedef BasicBufferFile<XWFile>		WBufferFile;


template <typename BufferFileT>
class FileInStream :public BufferFileT
{
public:
	typedef BufferFileT							BufferFileType;
	typedef typename BufferFileType::char_type	char_type;
	FileInStream() :rpos_(0) {}
	void Skip(size_t len)
	{
		if ((rpos_ + len) >= GetLength())
		{
			SetState(St_EOF);
			return;
		}
		rpos_ += len;
	}
	char_type Get()
	{
		if (rpos_ >= GetLength())
		{
			SetState(St_EOF);
			return 0;
		}
		char_type c = *(char_type*)(GetBuffer() + rpos_);
		rpos_ += CharSize();
		return c;
	}
	char_type Peek()
	{
		if (rpos_ >= GetLength())
		{
			SetState(St_EOF);
			return 0;
		}
		return *(char_type*)(GetBuffer() + rpos_);
	}
	size_t GetCount()
	{
		return GetLength() / CharSize();
	}
private:

	constexpr size_t CharSize()
	{
		return sizeof(char_type) / sizeof(char);
	}
private:
	size_t rpos_;
};
#endif // !FileIO_H_
