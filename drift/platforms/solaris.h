//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@


#if __GNUC__ >= 4
#define DSL_API __attribute__ ((visibility("default")))
#define DSL_API_CLASS __attribute__ ((visibility("default")))
#else
#define DSL_API
#define DSL_API_CLASS
#endif
#define DSL_CC

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
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dlfcn.h>

//platform/cpu defines
#define PLATFORM "Solaris"
#define SOLARIS
#ifdef __i386__
	#define CPU "x86"
#else
	#warning WARNING: Unknown CPU Type
#endif

//platform-specific types/defines
#define MAX_PATH 1024
#define THREADTYPE void *
typedef void * (*ThreadProto)(void *);

//crt fixups
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define dsl_mkdir(x,y) mkdir(x, y)
