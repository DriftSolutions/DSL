//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dslcore.h>
#include <drift/mutex.h>
#include <drift/threading.h>
#include <drift/GenLib.h>
#if defined(DSL_THREADING_USE_C11)
#define DSL_LIST_TYPE unordered_set
#include <unordered_set>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#else
#define DSL_LIST_TYPE set
#include <set>
#endif
#if defined(WIN32)
#include <process.h>
#endif

DSL_Mutex  * DSL_Thread_Mutex()
{
	static DSL_Mutex actualMutex;
	return &actualMutex;
}
typedef DSL_LIST_TYPE<DSL_THREAD_INFO *> DSL_List_Type;
DSL_List_Type DSL_List;
uint32 DSL_NoThreads=0;

void DSL_UnregisterThread(DSL_THREAD_INFO * tt) {
	AutoMutexPtr(DSL_Thread_Mutex());
	DSL_List_Type::iterator x = DSL_List.find(tt);
	if (x != DSL_List.end()) {
		DSL_List.erase(x);
	}
#if defined(WIN32)
	if (tt->hThread) {
		CloseHandle(tt->hThread);
	}
#endif
	dsl_free(tt);
	DSL_NoThreads--;
}

#ifdef WIN32
typedef struct tagTHREADNAME_INFO
{
  DWORD dwType; // must be 0x1000
  LPCSTR szName; // pointer to name (in user addr space)
  DWORD dwThreadID; // thread ID (-1=caller thread)
  DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

DSL_API void DSL_CC DSL_SetThreadName(DWORD dwThreadID, LPCSTR szThreadName) {
	if (szThreadName == NULL) { return; }
  THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try {
#ifdef _WIN64
    RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (const ULONG_PTR *)&info );
#else
    RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
#endif
  }
  __except (EXCEPTION_CONTINUE_EXECUTION) {
  }
}
#else
DSL_API void DSL_CC DSL_SetThreadName(pthread_t thread_id, const char * szThreadName) {
	if (szThreadName == NULL) { return; }
	pthread_setname_np(thread_id, szThreadName);
	/*
	#if defined(PR_SET_NAME)
	if (pthread_self() == thread_id) {
		prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
	}
#elif (defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__DragonFly__))
    pthread_set_name_np(pthread_self(), name);
#elif defined(MAC_OSX)
    pthread_setname_np(name);
#else
	*/
}
#endif

DSL_THREAD_INFO * DSL_CC DSL_StartThread(ThreadProto Thread,void * Parm, const char * desc, int32 id) {
	DSL_THREAD_INFO * ret = (DSL_THREAD_INFO *)dsl_malloc(sizeof(DSL_THREAD_INFO));
	memset(ret, 0, sizeof(DSL_THREAD_INFO));

	DSL_Thread_Mutex()->Lock();
	DSL_List.insert(ret);
	DSL_NoThreads++;
	DSL_Thread_Mutex()->Release();

	if (desc) { sstrcpy(ret->desc, desc); }
	ret->id = id;
	ret->parm = Parm;
	ret->RemoveMe = DSL_UnregisterThread;

#if defined(WIN32)
	unsigned int ThreadID=0;
	ret->hThread = (HANDLE)_beginthreadex(NULL, 0, Thread, ret, 0, &ThreadID);
	if (ret->hThread != 0) {
		DSL_SetThreadName(ThreadID, desc);
	} else {
		DSL_UnregisterThread(ret);
		return NULL;
	}
#else
	if (pthread_create(&ret->hThread, NULL, Thread, (void *)ret) == 0) {
		DSL_SetThreadName(ret->hThread, desc);
		pthread_detach(ret->hThread); // tells OS that it can reclaim used memory after thread exits
	} else {
		DSL_UnregisterThread(ret);
		return NULL;
	}
#endif

	return ret;
}

bool DSL_CC DSL_StartThreadNoRecord(ThreadProto Thread,void * Parm, const char * Desc) {
#if defined(WIN32)
	unsigned int ThreadID=0;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, Thread, Parm, 0, &ThreadID);
	if (hThread != 0) {
		DSL_SetThreadName(ThreadID, Desc);
		CloseHandle(hThread);
		return true;
	}
#else
	pthread_t a_thread;
	if (pthread_create(&a_thread, NULL, Thread, Parm) == 0) {
		DSL_SetThreadName(a_thread, Desc);
		pthread_detach(a_thread); // tells OS that it can reclaim used memory after thread exits
		return true;
	}
#endif
	return false;
}

bool DSL_CC DSL_KillThread(DSL_THREAD_INFO * tt) {
#if defined(WIN32)
	TerminateThread(tt->hThread, 0);
	DSL_UnregisterThread(tt);
	return true;
#else
	if (pthread_cancel(tt->hThread) == 0) {
		DSL_UnregisterThread(tt);
		return true;
	}
	return false;
#endif
}

void DSL_CC DSL_PrintRunningThreads() {
	AutoMutexPtr(DSL_Thread_Mutex());

	printf("Running threads:");
	for (DSL_List_Type::iterator x = DSL_List.begin(); x != DSL_List.end(); x++) {
		DSL_THREAD_INFO * tScan = *x;
		printf(" [%s]", tScan->desc);
	}
	printf("\n");
}

void DSL_CC DSL_PrintRunningThreadsWithID(int id) {
	AutoMutexPtr(DSL_Thread_Mutex());

	printf("Running threads:");
	for (DSL_List_Type::iterator x = DSL_List.begin(); x != DSL_List.end(); x++) {
		DSL_THREAD_INFO * tScan = *x;
		if (tScan->id == id) {
			printf(" [%s]",tScan->desc);
		}
	}
	printf("\n");
}

uint32 DSL_CC DSL_NumThreads() {
	AutoMutexPtr(DSL_Thread_Mutex());
	return DSL_NoThreads;
}

uint32 DSL_CC DSL_NumThreadsWithID(int id) {
	uint32 ret = 0;
	AutoMutexPtr(DSL_Thread_Mutex());
	for (DSL_List_Type::iterator x = DSL_List.begin(); x != DSL_List.end(); x++) {
		DSL_THREAD_INFO * tScan = *x;
		if (tScan->id == id) { ret++; }
	}
	return ret;
}

void DSL_CC safe_sleep(int sleepfor, bool inmilli) {
#if defined(DSL_THREADING_USE_C11)
	this_thread::sleep_for(inmilli ? chrono::milliseconds(sleepfor) : chrono::seconds(sleepfor));
#elif defined(WIN32)
	if (!inmilli) {
		Sleep(1000 * sleepfor);
	} else {
		Sleep(sleepfor);
	}
#else // Unix-style
	struct timespec delta, rem;
	if (!inmilli) {
		delta.tv_sec = sleepfor;
		delta.tv_nsec= 0;
	} else {
		delta.tv_sec = sleepfor / 1000;
		sleepfor -= (delta.tv_sec * 1000);
		delta.tv_nsec= sleepfor * 1000000;
	}

	while (nanosleep(&delta,&rem) == -1 && errno == EINTR) {
		memcpy(&delta, &rem, sizeof(rem));
	}
#endif
}
