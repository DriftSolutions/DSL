//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#include <inttypes.h>

#if !defined(__BOOL_DEFINED) && !defined(__cplusplus)
#define bool unsigned char
#endif

#if (defined(_MSC_VER) && _MSC_VER < 1600)
	typedef signed __int64 int64;
	typedef unsigned __int64 uint64;
	typedef signed __int32 int32;
	typedef unsigned __int32 uint32;
	typedef signed __int16 int16;
	typedef unsigned __int16 uint16;
	typedef signed __int8 int8;
	typedef unsigned __int8 uint8;
#else
	#include <stdint.h>
	typedef int64_t int64;
	typedef uint64_t uint64;
	typedef int32_t int32;
	typedef uint32_t uint32;
	typedef int16_t int16;
	typedef uint16_t uint16;
	typedef int8_t int8;
	typedef uint8_t uint8;
#endif

#define I64FMT "%" PRId64 ""
#define U64FMT "%" PRIu64 ""

#if defined(WIN32)
	typedef int socklen_t;
	#define atoi64(x) _atoi64(x)
	#define atou64(x) _strtoui64(x, NULL, 10)
	#define atoul(x) strtoul(x, NULL, 10)
#else
	#define VOID void
	#ifndef SOCKET
		#define SOCKET int
	#endif
	#define BYTE unsigned char
	#ifndef HANDLE
		#define HANDLE void *
	#endif
	#define atoi64(x) strtoll(x, NULL, 10)
	#define atou64(x) strtoull(x, NULL, 10)
	#define atoul(x) strtoul(x, NULL, 10)
#endif

#define sstrcpy(x,y) strlcpy(x,y,sizeof(x))
#define sstrcat(x,y) strlcat(x,y,sizeof(x))

#if defined(WIN32)
	#define PATH_SEP '\\'
	#define PATH_SEPS "\\"
	#define WPATH_SEPS L"\\"
#else
	#define PATH_SEP '/'
	#define PATH_SEPS "/"
	#define WPATH_SEPS L"/"
#endif

typedef wchar_t wchar;
#if defined(UNICODE) || defined(_UNICODE)
typedef wchar_t tchar;
#define TPATH_SEPS WPATH_SEPS
#define tstring wstring
#define tstat _wstat
#define tstatstruct _stat64i32
#define taccess _waccess
#define tgetcwd _wgetcwd
#define tremove _wremove
#define ttoi _tstoi
#define tcsstr _tcsstr
#else
typedef char tchar;
#define TPATH_SEPS PATH_SEPS
#define tstring string
#define tstat stat
#define tstatstruct stat
#define taccess _access
#define tremove remove
#define tgetcwd getcwd
#define ttoi atoi
#endif

#if defined(WIN32)
//the eternal CRT fixup
#define tcslen _tcslen
#define tcscpy _tcscpy
#define tcsncpy _tcsncpy
#define tcscat _tcscat
#define tprintf _tprintf
#define tsnprintf _sntprintf
#define tcschr _tcschr
#define tcsrchr _tcsrchr
#define stprintf _stprintf
#define tstoi _tstoi
#define tcscmp tcscmp
#define tcsicmp _tcsicmp
#define tcsncmp _tcsncmp
#define tcsnicmp _tcsnicmp
#endif

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
COMPILE_TIME_ASSERT(sizeof(int64) == 8)
COMPILE_TIME_ASSERT(sizeof(uint64) == 8)
COMPILE_TIME_ASSERT(sizeof(int32) == 4)
COMPILE_TIME_ASSERT(sizeof(uint32) == 4)
COMPILE_TIME_ASSERT(sizeof(int16) == 2)
COMPILE_TIME_ASSERT(sizeof(uint16) == 2)
COMPILE_TIME_ASSERT(sizeof(int8) == 1)
COMPILE_TIME_ASSERT(sizeof(uint8) == 1)
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
#pragma GCC diagnostic pop
#endif

#if  defined(__i386__) || defined(__ia64__) || defined(WIN32) || (defined(__alpha__) || defined(__alpha)) || defined(__arm__) || defined(ARM) || (defined(__mips__) && defined(__MIPSEL__)) || defined(__SYMBIAN32__) || defined(__x86_64__) || defined(__LITTLE_ENDIAN__)
	#undef BIG_ENDIAN
	#ifndef LITTLE_ENDIAN
	#define LITTLE_ENDIAN
	#endif
#else
	#undef LITTLE_ENDIAN
	#ifndef BIG_ENDIAN
	#define BIG_ENDIAN
	#endif
#endif
