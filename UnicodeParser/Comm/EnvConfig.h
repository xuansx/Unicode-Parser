#pragma once
#ifndef EnvConfig_H__
#define EnvConfig_H__

#if defined(__GNUC__)
#define GNUC	1
#endif

#ifdef _MSC_VER
#define MSVC	1

#endif // _MSVC_Ver

#if defined(__ANDROID__)
# define OS_ANDROID 1
# define OS_LINUX 1
# define OS_POSIX 1
#elif defined(__APPLE__)
# define OS_BSD 1
# define OS_MACOSX 1
# define OS_POSIX 1
#elif defined(__CYGWIN__)
# define OS_CYGWIN 1
# define OS_POSIX 1
#elif defined(__linux__)
# define OS_LINUX 1
# define OS_POSIX 1
#elif defined(__FreeBSD__)
# define OS_BSD 1
# define OS_FREEBSD 1
# define OS_POSIX 1
#elif defined(__NetBSD__)
# define OS_BSD 1
# define OS_NETBSD 1
# define OS_POSIX 1
#elif defined(__OpenBSD__)
# define OS_BSD 1
# define OS_OPENBSD 1
# define OS_POSIX 1
#elif defined(_WIN32)
# define OS_WIN 1
#endif


#endif // !EnvConfig_H__
