//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

// int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#if defined(COMCTL32_V6)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#if !defined(DSL_DLL) && !defined(DSL_STATIC)
#pragma message( "Neither DSL_DLL or DSL_STATIC defined, defaulting to DSL_STATIC!" )
#define DSL_STATIC
#endif

#define PART1 ""

#if defined(DSL_STATIC)
#define PART2 "-static"
#else
#define PART2 ""
#endif

#if defined(DEBUG)
#define LIBSUFFIX "_d"
#else
#define LIBSUFFIX ""
#endif

#ifndef DSL_EXPORTS
#define LIBNAME "dsl" PART1 "-core" PART2 LIBSUFFIX ".lib"
#pragma message( "Will automatically link with " LIBNAME "" )
#pragma comment(lib, LIBNAME)
#endif
#if defined(ENABLE_CURL) && !defined(DSL_CURL_EXPORTS)
#define LIBNAMECURL "dsl" PART1 "-curl" PART2 LIBSUFFIX ".lib"
#pragma message( "Will automatically link with " LIBNAMECURL "" )
#pragma comment(lib, LIBNAMECURL)
#endif
#if defined(ENABLE_SQLITE) && !defined(DSL_SQLITE_EXPORTS)
#define LIBNAMESQLITE "dsl" PART1 "-sqlite" PART2 LIBSUFFIX ".lib"
#pragma message( "Will automatically link with " LIBNAMESQLITE "" )
#pragma comment(lib, LIBNAMESQLITE)
#endif
#if defined(ENABLE_MYSQL) && !defined(DSL_MYSQL_EXPORTS)
#define LIBNAMEMYSQL "dsl" PART1 "-mysql" PART2 LIBSUFFIX ".lib"
#pragma message( "Will automatically link with " LIBNAMEMYSQL "" )
#pragma comment(lib, LIBNAMEMYSQL)
#endif
#if defined(ENABLE_PHYSFS) && !defined(DSL_PHYSFS_EXPORTS)
#define LIBNAMEPHYSFS "dsl" PART1 "-physfs" PART2 LIBSUFFIX ".lib"
#pragma message( "Will automatically link with " LIBNAMEPHYSFS "" )
#pragma comment(lib, LIBNAMEPHYSFS)
#endif
#if defined(ENABLE_OPENSSL) && !defined(DSL_OPENSSL_EXPORTS)
#define LIBNAMEOPENSSL "dsl" PART1 "-openssl" PART2 LIBSUFFIX ".lib"
#pragma message( "Will automatically link with " LIBNAMEOPENSSL "" )
#pragma comment(lib, LIBNAMEOPENSSL)
#endif
#if defined(ENABLE_GNUTLS) && !defined(DSL_GNUTLS_EXPORTS)
#define LIBNAMEGNUTLS "dsl" PART1 "-gnutls" PART2 LIBSUFFIX ".lib"
#pragma message( "Will automatically link with " LIBNAMEGNUTLS "" )
#pragma comment(lib, LIBNAMEGNUTLS)
#endif
#if defined(ENABLE_SODIUM) && !defined(DSL_SODIUM_EXPORTS)
#define LIBNAMESODIUM "dsl" PART1 "-sodium" PART2 LIBSUFFIX ".lib"
#pragma message( "Will automatically link with " LIBNAMESODIUM "" )
#pragma comment(lib, LIBNAMESODIUM)
#endif

#if defined(DSL_DLL)
	#if defined(DSL_EXPORTS)
		#define DSL_API extern "C" __declspec(dllexport)
		#define DSL_API_CLASS __declspec(dllexport)
	#else
		#define DSL_API extern "C" __declspec(dllimport)
		#define DSL_API_CLASS __declspec(dllimport)
	#endif
#else
	#define DSL_API
	#define DSL_API_CLASS
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
//#ifndef _CRT_NON_CONFORMING_SWPRINTFS
//#define _CRT_NON_CONFORMING_SWPRINTFS
//#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#pragma warning(disable: 4251)
#pragma warning(disable: 4800) //'int' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4995)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif


//headers
#include <Ws2tcpip.h>
//#include <winsock2.h>
#include <Wspiapi.h>
#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

#include <wchar.h>
#include <tchar.h>
#include <limits.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <shlobj.h>

#if defined(NTDDI_WIN10_RS4)
#include <afunix.h>
#else
#define UNIX_PATH_MAX 108

typedef struct sockaddr_un
{
	ADDRESS_FAMILY sun_family;     /* AF_UNIX */
	char sun_path[UNIX_PATH_MAX];  /* pathname */
} SOCKADDR_UN, *PSOCKADDR_UN;
#endif

//libs
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")

#if defined(ENABLE_GNUTLS)
#pragma comment(lib, "libgnutls-30.lib")
#endif

#if defined(ENABLE_OPENSSL)
#if defined(DEBUG_SSL)
#pragma comment(lib, "libssl32_d.lib")
#pragma comment(lib, "libcrypto32_d.lib")
#else
#pragma comment(lib, "libssl32.lib")
#pragma comment(lib, "libcrypto32.lib")
#endif
#endif

#if defined(ENABLE_MYSQL)
#if defined(DEBUG_MYSQL)
#pragma comment(lib, "libmariadb_d.lib")
#else
#pragma comment(lib, "libmariadb.lib")
#endif
//#pragma comment(lib, "libmysql.lib")
#endif

#if defined(ENABLE_SQLITE)
	#if defined(DEBUG)
		#pragma comment(lib, "sqlite3_d.lib")
	#else
		#pragma comment(lib, "sqlite3.lib")
	#endif
#endif

#if defined(ENABLE_ZLIB)
	#if defined(DEBUG)
		#pragma comment(lib, "zlib-static_d.lib")
	#else
		#pragma comment(lib, "zlib-static.lib")
	#endif
#endif

#if defined(ENABLE_CURL)
	#if defined(DEBUG)
		#pragma comment(lib, "libcurl_d.lib")
	#else
		#pragma comment(lib, "libcurl.lib")
	#endif
#endif

#if defined(ENABLE_PHYSFS)
	#if defined(DEBUG)
		#pragma comment(lib, "physfs_d.lib")
	#else
		#pragma comment(lib, "physfs.lib")
	#endif
#endif

#if defined(ENABLE_SODIUM)
	#pragma comment(lib, "libsodium.lib")
#endif

//platform/cpu defines
#if defined(_WIN64) || defined(WIN64)
	#if !defined(WIN64)
	#define WIN64
	#endif
	#define PLATFORM "Win64"
	#define CPU "x64"
	/* __stdcall has no effect on 64-bit Windows */
	#define DSL_CC
#else
	#define PLATFORM "Win32"
	#define CPU "x86"
	#define DSL_CC __stdcall
#endif

#include <drift/win32/poll.h>

//platform-specific types/defines
#define THREADTYPE unsigned __stdcall
typedef unsigned (__stdcall *ThreadProto)(void *);
#if !defined(EWOULDBLOCK) || EWOULDBLOCK != WSAEWOULDBLOCK
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif
#ifndef ENOTCONN
#define ENOTCONN WSAENOTCONN
#endif
#ifndef EINPROGRESS
#define EINPROGRESS WSAEINPROGRESS
#endif
#define R_OK 0x04
#define W_OK 0x02
#define F_OK 0x00

//crt fixups
#define snprintf _snprintf
#define wfopen _wfopen
#define getcwd _getcwd
#define snwprintf _snwprintf
#define vscprintf _vscprintf
#define vscwprintf _vscwprintf
#define wgetenv _wgetenv
#define access _access
#define stricmp _stricmp
#define S_ISDIR(x) (x & _S_IFDIR)
#define S_ISREG(x) (x & _S_IFREG)
#define S_ISEXEC(x) (x & _S_IEXEC)
#define S_ISLNK(x) (0)
#define stat64 _stati64
#define lseek64 _lseeki64
#define tell64 _telli64
#define ftruncate _chsize_s
//#define getpid GetCurrentProcessId
//#define stat64struct _stat64
#define dsl_mkdir(x,y) _mkdir(x)
