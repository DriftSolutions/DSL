//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef _DSL_COMPAT_H_
#define _DSL_COMPAT_H_

#define fill_random_buffer dsl_fill_random_buffer

#define TITUS_SOCKET DSL_SOCKET
#define T_SOCKET D_SOCKET
#define TITUS_SOCKET_LIST DSL_SOCKET_LIST
#define TFD_ZERO DFD_ZERO
#define TFD_SET DFD_SET
#define TFD_ISSET DFD_ISSET
#define Titus_Sockets3 DSL_Sockets3
#define Titus_Sockets DSL_Sockets
#define TS3_FLAG_SSL DS3_FLAG_SSL
#define TS3_FLAG_ZIP DS3_FLAG_ZIP
#define TS3_SSL_METHOD_TLS DS3_SSL_METHOD_TLS
#define TS3_SSL_METHOD_TLS1_2 DS3_SSL_METHOD_TLS1_2
#define TS3_SSL_METHOD_TLS1_1 DS3_SSL_METHOD_TLS1_1
#define TS3_SSL_METHOD_TLS1_0 DS3_SSL_METHOD_TLS1_0
#define TS3_SSL_METHOD_DEFAULT DS3_SSL_METHOD_DEFAULT

#define Titus_Buffer DSL_Buffer
#define Titus_Mutex DSL_Mutex
#define Titus_TimedMutex DSL_TimedMutex
#define Titus_Registry DSL_Registry
#define Titus_Download DSL_Download
#define TitusDownloadNoCurl DSL_Download_NoCurl
#define TitusDownloadCurl DSL_Download_Curl

#define TT_THREAD_INFO DSL_THREAD_INFO

#define TT_StartThread DSL_StartThread
#define TT_StartThreadNoRecord DSL_StartThreadNoRecord
#define SetThreadName DSL_SetThreadName
#define TT_NumThreads DSL_NumThreads
#define TT_NumThreadsWithID DSL_NumThreadsWithID
#define TT_PrintRunningThreads DSL_PrintRunningThreads
#define TT_PrintRunningThreadsWithID DSL_PrintRunningThreadsWithID
#define TT_KillThread DSL_KillThread

#define TT_DEFINE_THREAD  DSL_DEFINE_THREAD

#define TT_THREAD_START DSL_THREAD_START
#define TT_THREAD_END DSL_THREAD_END

#define TT_THREADEND DSL_THREADEND

#endif // _DSL_COMPAT_H_
