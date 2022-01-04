//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#if __GNUC__ >= 4 && (defined(DSL_SHARED) || defined(DSL_DLL)) && defined(HAVE_VIS)
#define DSL_API __attribute__ ((visibility("default")))
#define DSL_API_CLASS __attribute__ ((visibility("default")))
#else
#define DSL_API
#define DSL_API_CLASS
#endif
#define DSL_CC

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define DSL_DEPRECATE  __attribute__((__deprecated__))
#else
#define DSL_DEPRECATE
#endif /* __GNUC__ */

#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS != 64
#define _FILE_OFFSET_BITS 64
#endif

// Older versions of GCC won't define the format macros without this
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

//headers
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <limits.h>

#include <new>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/un.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <wchar.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <syscall.h>
#include <linux/random.h>
#if (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 25))	
#include <sys/random.h>
#endif

//platform/cpu defines
#define PLATFORM "Linux"
#define LINUX
#if defined(__x86_64__)
	#define CPU "x86_64"
	#define DSL_HAVE_CPUID
#elif defined(__i386__)
	#define CPU "x86"
	#define DSL_HAVE_CPUID
#elif defined(__arm__)
	#define CPU "ARM"
#else
	#warning WARNING: Unknown CPU Type
#endif

//platform-specific types/defines
#define MAX_PATH 1024
#define THREADTYPE void *
typedef void * (*ThreadProto)(void *);

//crt fixups
/**
 * \defgroup crtlinux Linux CRT Fixups
 */

/** \addtogroup crtlinux
 * @{
 */

#define strtol64(x,y,z) strtoll(x,y,z)
#define strtoul64(x,y,z) strtoull(x,y,z)
#define atoi64(x) strtol64(x, NULL, 10)
#define atou64(x) strtoul64(x, NULL, 10)
#define atoul(x) strtoul(x, NULL, 10)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define stristr strcasestr
#define snwprintf swprintf
#define dsl_mkdir(x,y) mkdir(x, y)
//#define lseek64 lseek64
#define tell64(x) lseek64(x, 0, SEEK_CUR)

/**@}*/

COMPILE_TIME_ASSERT(sizeof(off_t) == 8)
