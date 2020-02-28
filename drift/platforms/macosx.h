//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!


#if __GNUC__ >= 4
#define DSL_API __attribute__ ((visibility("default")))
#define DSL_API_CLASS __attribute__ ((visibility("default")))
#else
#define DSL_API
#define DSL_API_CLASS
#endif
#define DSL_CC

//headers
#include <sys/cdefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>

#include <new>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <dlfcn.h>

//platform/cpu defines
#define PLATFORM "MacOSX"
#ifndef MACOSX
#define MACOSX
#endif
#ifdef __i386__
	#define CPU "x86"
#elif __amd64__
	#define CPU "x86_64"
#else
	#error "ERROR: Unknown CPU Type"
#endif

//platform-specific types/defines
#define MAX_PATH 1024
#define THREADTYPE void *
#define PF_INET AF_INET
#define PF_UNSPEC AF_UNSPEC
typedef void * (*ThreadProto)(void *);

//crt fixups
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define stristr strcasestr
#define snwprintf swprintf
#define dsl_mkdir(x,y) mkdir(x, y)
DSL_API wchar_t * DSL_CC wcsdup(const wchar_t *s);
