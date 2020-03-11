//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef _DSL_THREADING_H_
#define _DSL_THREADING_H_

#if defined(__cplusplus) && ((defined(_MSC_VER) && _MSC_VER >= 1800 && __cplusplus >= 199711L) || __cplusplus >= 201103L)
#define DSL_THREADING_USE_C11
#endif

/**
 * \defgroup threads Threads
 */

/** \addtogroup threads
 * @{
 */

#if defined(WIN32)
#define THREADIDTYPE DWORD
#else
#define THREADIDTYPE pthread_t
#endif

struct DSL_THREAD_INFO {
#ifdef _WIN32
	HANDLE hThread;
#else
	pthread_t hThread;
#endif

	int32 id; ///< User-specified ID, if DSL_StartThread() is used with no ID, it defaults to -1
	void * parm; ///< User-specified parameter

	char desc[256];

#ifndef DOXYGEN_SKIP
	void (*RemoveMe)(DSL_THREAD_INFO * tt);
#endif
};

#if !defined(NO_CPLUSPLUS) || defined(DOXYGEN_SKIP)
/**
 * Starts a thread. Declare and define your thread with DSL_DEFINE_THREAD(Name)
 */
DSL_API DSL_THREAD_INFO * DSL_CC DSL_StartThread(ThreadProto Thread, void * Parm, const char * desc = NULL, int32 id = -1);
#else
DSL_API DSL_THREAD_INFO * DSL_CC DSL_StartThread(ThreadProto Thread, void * Parm, const char * desc, int32 id);
#endif
/**
 * Starts a thread with no tracking structure.
 */
DSL_API bool DSL_CC DSL_StartThreadNoRecord(ThreadProto Thread, void * Parm);

DSL_API uint32 DSL_CC DSL_NumThreads(); ///< Number of active threads (does not count threads started with DSL_StartThreadNoRecord)
DSL_API uint32 DSL_CC DSL_NumThreadsWithID(int id); ///< Number of active threads with matching id #
DSL_API void DSL_CC DSL_PrintRunningThreads();
DSL_API void DSL_CC DSL_PrintRunningThreadsWithID(int id);
DSL_API bool DSL_CC DSL_KillThread(DSL_THREAD_INFO * tt);

#define DSL_DEFINE_THREAD(x) THREADTYPE x(void * lpData)

#if defined(WIN32)
//new-style
#define DSL_THREAD_START DSL_THREAD_INFO * tt = (DSL_THREAD_INFO *)lpData;
#define DSL_THREAD_END tt->RemoveMe(tt); return 0;

#if !defined(DSL_NO_OLD_THREAD_API) && !defined(DOXYGEN_SKIP)
//old-style
#define DSL_THREADEND return 0;
#endif

DSL_API void DSL_CC DSL_SetThreadName(DWORD dwThreadID, LPCSTR szThreadName);
#else
//new-style
#define DSL_THREAD_START DSL_THREAD_INFO * tt = (DSL_THREAD_INFO *)lpData;
#define DSL_THREAD_END tt->RemoveMe(tt); return NULL;

#if !defined(DSL_NO_OLD_THREAD_API) && !defined(DOXYGEN_SKIP)
//old-style
#define DSL_THREADEND return NULL;
#endif

#define GetCurrentThreadId pthread_self
#endif

#if !defined(NO_CPLUSPLUS) || defined(DOXYGEN_SKIP)
DSL_API void DSL_CC safe_sleep(int sleepfor, bool inmilli = false);
#else
DSL_API void DSL_CC safe_sleep(int sleepfor, bool inmilli);
#endif
#define safe_sleep_ms(sleepfor) safe_sleep(sleepfor, true)
#define safe_sleep_s(sleepfor) safe_sleep(sleepfor, false)

/**@}*/

#endif // _DSL_MUTEX_H_
