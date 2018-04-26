
#include "FileIO.h"
#include "EnvConfig.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>

void	MakeDir(const char* path)
{
	if (!path)
	{
		return;
	}
	if (strncmp(path, "./", 2) == 0)
	{
		return;
	}
	if (_access(path, 0) == -1)
	{
		_mkdir(path);
	}
}
int			DelFile(const char* filename)
{
	return remove(filename);
}
/*
r	以只读方式打开文件，该文件必须存在。
r+	以读/写方式打开文件，该文件必须存在。
rb+	以读/写方式打开一个二进制文件，只允许读/写数据。
rt+	以读/写方式打开一个文本文件，允许读和写。
w	打开只写文件，若文件存在则长度清为 0，即该文件内容消失，若不存在则创建该文件。
w+	打开可读/写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立该文件。
a	以附加的方式打开只写文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾，即文件原先的内容会被保留（EOF 符保留）。
a+	以附加方式打开可读/写的文件。若文件不存在，则会建立该文件，如果文件存在，则写入的数据会被加到文件尾后，即文件原先的内容会被保留（原来的 EOF 符不保留）。
wb	以只写方式打开或新建一个二进制文件，只允许写数据。
wb+	以读/写方式打开或建立一个二进制文件，允许读和写。
wt+	以读/写方式打开或建立一个文本文件，允许读写。
at+	以读/写方式打开一个文本文件，允许读或在文本末追加数据。
ab+	以读/写方式打开一个二进制文件，允许读或在文件末追加数据。
*/

#define FILE_OP_STR(C)\
C##"r",\
C##"r+",\
C##"rb+",\
C##"rt+",\
C##"w",\
C##"w+",\
C##"a",\
C##"a+",\
C##"wb",\
C##"wb+",\
C##"wt+",\
C##"at+",\
C##"ab+"

//struct XFile :public FileIOState {

const char* XFile::FileOpStr[XFile::Op_Count] =
{
#ifndef T_sim_
#define T_sim_
	FILE_OP_STR(T_sim_)
#undef  T_sim_
#endif // 

};
void XFileBase::ResetSeek()
{
	if (!handle() || Good())
	{
		return;
	}
	fseek(handle(), 0, SEEK_SET);
}

size_t XFileBase::Size()
{
	struct stat filestats;
	if (!handle() && !Good())return 0;
	int fd = _fileno(handle());
	fstat(fd, &filestats);
	return filestats.st_size;
}
void XFileBase::closeFile()
{
	if (handle_)
	{
		fclose(handle_);
		handle_ = 0;
	}
}
bool XFileBase::Write(const void* buffer, size_t length)
{
	if (!handle() || !Good())
	{
		return false;
	}
	size_t start = 0;
	size_t bytes_write;
	{
		do
		{
			bytes_write = fwrite((char*)buffer + start, 1, length - start, handle());
			start += bytes_write;
		} while (start < length);
		return true;
	}
	return true;
}
bool XFileBase::Read(void* buffer, size_t& length)
{
	if (!handle() || !Good())
	{
		return false;
	}
	memset(buffer, 0, length);
	size_t start = 0;
	size_t bytes_read;
	do
	{
		bytes_read = fread((char*)buffer + start, 1, length - start, handle());
		if (bytes_read == 0)
			break;
		start += bytes_read;
	} while (start < length);
	return true;
}

bool XFile::Open(const char* filename, eFileOp op)
{
	closeFile();
#ifdef MSVC
	if (fopen_s(handlePtr(), filename, FileOpStr[op]) != 0)
	{
		SetState(St_IOError);
		return false;
	}
#else
	handle(fopen(filename, FileOpStr[op]));
#endif // MSVC
	SetState(St_OK);
	return true;
}

const wchar_t* XWFile::WFileOpStr[Op_Count] =
{
	FILE_OP_STR(L)
};
bool XWFile::Open(const wchar_t* filename, eFileOp op, eCodeType code)
{
	closeFile();
	wchar_t mode[32] = {};
#ifdef MSVC
	wcscat_s(mode, WFileOpStr[op]);
#else
	wcscat(mode, WFileOpStr[op]);
#endif // MSVC
	
	switch (code)
	{
	case WFileState::CT_NONE:
		break;
	case WFileState::CT_UTF8:
#ifdef MSVC
		wcscat_s(mode, L", ccs=UTF-8");
#else
		wcscat(mode, L", ccs=UTF-8");
#endif // MSVC		
		break;
	case WFileState::CT_UNICODE:
#ifdef MSVC
		wcscat_s(mode, L", ccs=UNICODE");
#else
		wcscat(mode, L", ccs=UNICODE");
#endif // MSVC	
		break;
	default:
		break;
	}
#ifdef MSVC
	if (_wfopen_s(handlePtr(), filename, mode) != 0)
	{
		SetState(St_IOError);
		return false;
	}
#else
	handle(_wfopen(filename, mode));
#endif // MSVC
	SetState(St_OK);
	return true;
}
