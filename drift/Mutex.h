//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef _INCLUDE_MUTEX_H_
#define _INCLUDE_MUTEX_H_

#include <drift/Threading.h>

/**
 * \defgroup mutex Mutexes/Critical Sections
 * DSL_Mutex will be #define'd automatically to the right implementation for your system.
 */

/** \addtogroup mutex
 * @{
 */

#if !defined(DSL_DEFAULT_MUTEX_TIMEOUT)
#define DSL_DEFAULT_MUTEX_TIMEOUT -1
#endif

/**
 * Base class for our mutex implementations.
 */
class DSL_API_CLASS DSL_Mutex_Base {
public:
	virtual bool Lock(int timeout)=0;
	virtual bool Lock()=0;
	virtual void Release()=0;

	virtual void SetLockTimeout(int timeout)=0;
	virtual bool IsLocked()=0;
	virtual bool IsLockMine()=0;

	virtual THREADIDTYPE LockingThread()=0;
};

#ifndef DOXYGEN_SKIP

#if defined(WIN32) || defined(XBOX)

class DSL_API_CLASS DSL_Mutex_Win32CS : public DSL_Mutex_Base {
	private:
		THREADIDTYPE LockingThreadID;
		int refcnt;
		int lock_timeout;
		CRITICAL_SECTION cs;
	public:
		DSL_Mutex_Win32CS(int timeout = DSL_DEFAULT_MUTEX_TIMEOUT);
		~DSL_Mutex_Win32CS();

		bool Lock(int timeout);
		bool Lock();
		void Release();

		void SetLockTimeout(int timeout) { // -1 = no timeout
			lock_timeout = timeout;
		}
		bool IsLocked();
		bool IsLockMine();

		THREADIDTYPE LockingThread();
};

class DSL_API_CLASS DSL_Mutex_Win32Mutex : public DSL_Mutex_Base {
	private:
		THREADIDTYPE LockingThreadID;
		int refcnt;
		int lock_timeout;
		HANDLE hMutex;
	public:
		DSL_Mutex_Win32Mutex(int timeout = DSL_DEFAULT_MUTEX_TIMEOUT, const char * name = NULL);
		~DSL_Mutex_Win32Mutex();

		bool Lock(int timeout);
		bool Lock();
		void Release();

		void SetLockTimeout(int timeout) { // -1 = no timeout
			lock_timeout = timeout;
		}
		bool IsLocked();
		bool IsLockMine();

		THREADIDTYPE LockingThread();
};

#if DSL_DEFAULT_MUTEX_TIMEOUT > 0
#define DSL_Mutex DSL_Mutex_Win32Mutex
#define DSL_TimedMutex DSL_Mutex_Win32Mutex
#else
#define DSL_Mutex DSL_Mutex_Win32CS
#define DSL_TimedMutex DSL_Mutex_Win32Mutex
#endif

#else // defined(WIN32) || defined(XBOX)

class DSL_API_CLASS DSL_Mutex_pthreads : public DSL_Mutex_Base {
	private:
		THREADIDTYPE LockingThreadID;
		int refcnt;
		int lock_timeout;
		pthread_mutex_t hMutex;
	public:
		DSL_Mutex_pthreads(int timeout = DSL_DEFAULT_MUTEX_TIMEOUT);
		virtual ~DSL_Mutex_pthreads();

		bool Lock(int timeout);
		bool Lock();
		void Release();

		void SetLockTimeout(int timeout) { // -1 = no timeout
			lock_timeout = timeout;
		}
		bool IsLocked();
		bool IsLockMine();

		THREADIDTYPE LockingThread();
};

#define DSL_Mutex DSL_Mutex_pthreads
#define DSL_TimedMutex DSL_Mutex_pthreads
#endif // defined(WIN32) || defined(XBOX)

#endif // DOXYGEN_SKIP

#ifdef DEBUG_MUTEX
#define LockMutex(x) { \
	OutputDebugString("DSL_Mutex::Lock()\n"); \
	printf("DSL_Mutex::Lock(%s, %d)\n", __FILE__, __LINE__); \
	x.Lock(); \
	OutputDebugString("DSL_Mutex::Locked()\n"); \
	printf("DSL_Mutex::Locked(%s, %d)\n", __FILE__, __LINE__); \
}
#define LockMutexPtr(x) { \
	OutputDebugString("DSL_Mutex::Lock()\n"); \
	printf("DSL_Mutex::Lock(%s, %d)\n", __FILE__, __LINE__); \
	x->Lock(); \
	OutputDebugString("DSL_Mutex::Locked()\n"); \
	printf("DSL_Mutex::Locked(%s, %d)\n", __FILE__, __LINE__); \
}
#define RelMutex(x) { printf("DSL_Mutex::Release(%s, %d)\n", __FILE__, __LINE__); x.Release(); }
#define RelMutexPtr(x) { OutputDebugString("DSL_Mutex::Release()\n"); printf("DSL_Mutex::Release(%s, %d)\n", __FILE__, __LINE__); x->Release(); printf("DSL_Mutex::Released(%s, %d)\n", __FILE__, __LINE__); OutputDebugString("DSL_Mutex::Released()\n");  }
#else
#define LockMutex(x) x.Lock()
#define LockMutexPtr(x) x->Lock()
#define TryLockMutex(x, y) x.Lock(y)
#define TryLockMutexPtr(x, y) x->Lock(y)
#define RelMutex(x) x.Release()
#define RelMutexPtr(x) x->Release()
#endif

/**
 * Mutex locker class for automatic locking/unlocking in a scope.
 */
class DSL_MutexLocker {
private:
#ifdef DEBUG_MUTEX
	string fn;
	int line;
#endif
	DSL_Mutex_Base * hMutex;
public:
#ifdef DEBUG_MUTEX
	DSL_MutexLocker(DSL_Mutex_Base * mutex, string pfn = __FILE__, int pline = __LINE__) {
		hMutex = mutex;
		fn = pfn;
		line = pline;
		OutputDebugString("DSL_Mutex::Lock()\n");
		printf("DSL_Mutex::Lock(%s, %d)\n", fn.c_str(), line);
		hMutex->Lock();
		OutputDebugString("DSL_Mutex::Locked()\n");
		printf("DSL_Mutex::Locked(%s, %d)\n", fn.c_str(), line);
	}
	~DSL_MutexLocker() {
		OutputDebugString("DSL_Mutex::Release()\n");
		printf("DSL_Mutex::Release(%s, %d)\n", fn.c_str(), line);
		if (hMutex->IsLockMine()) {
			hMutex->Release();
		}
		printf("DSL_Mutex::Released(%s, %d)\n", fn.c_str(), line);
		OutputDebugString("DSL_Mutex::Released()\n");
	}
#else
	DSL_MutexLocker(DSL_Mutex_Base * mutex) {
		hMutex = mutex;
		LockMutexPtr(mutex);
	}
	~DSL_MutexLocker() {
		if (hMutex->IsLockMine()) {
			RelMutexPtr(hMutex);
		}
	}
#endif
};

#ifdef DEBUG_MUTEX
#define AutoMutex(x) DSL_MutexLocker MAKE_UNIQUE_NAME (&x, __FILE__, __LINE__)
#define AutoMutexPtr(x) DSL_MutexLocker MAKE_UNIQUE_NAME (x, __FILE__, __LINE__)
#else
#define AutoMutex(x) DSL_MutexLocker MAKE_UNIQUE_NAME (&x)
#define AutoMutexPtr(x) DSL_MutexLocker MAKE_UNIQUE_NAME (x)
#endif

/**@}*/

#ifndef DOXYGEN_SKIP

// *** DO NOT USE THIS, FOR DSL INTERNAL USE ONLY ***
DSL_Mutex * dslMutex();

#endif // DOXYGEN_SKIP

#endif // _INCLUDE_MUTEX_H_
