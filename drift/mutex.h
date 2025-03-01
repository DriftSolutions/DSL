//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2025 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef _INCLUDE_MUTEX_H_
#define _INCLUDE_MUTEX_H_

#include <drift/threading.h>
#include <chrono>
#include <mutex>

/**
 * \defgroup mutex Mutexes
 */

/** \addtogroup mutex
 * @{
 */

#if !defined(DSL_DEFAULT_MUTEX_TIMEOUT)
#define DSL_DEFAULT_MUTEX_TIMEOUT -1
#endif

/**
 * Cross-platform Mutex (now just a wrapper for C++11's recursive_timed_mutex)
 */
class DSL_API_CLASS DSL_Mutex {
private:
	recursive_timed_mutex hMutex;
	int lock_timeout = 0;
public:
	/**
	* timeout is the default timeout for Lock(), anything less than 0 for infinite blocking until locked
	*/
	DSL_Mutex(int timeout = DSL_DEFAULT_MUTEX_TIMEOUT);

	bool Lock(int timeout);
	bool Lock();
	void Release();

	void SetLockTimeout(int timeout) { // anything less than 0 for infinite blocking until locked. This should only be called before the mutex is being used or after the lock is already held, otherwise the behaviour is undefined.
		lock_timeout = timeout;
	}
};

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
	DSL_Mutex * hMutex;
	bool _locked;
public:
	const bool& locked = _locked;
#ifdef DEBUG_MUTEX
	DSL_MutexLocker(DSL_Mutex * mutex, const string& pfn = __FILE__, int pline = __LINE__) {
		hMutex = mutex;
		fn = pfn;
		line = pline;
		OutputDebugString("DSL_Mutex::Lock()\n");
		printf("DSL_Mutex::Lock(%s, %d)\n", fn.c_str(), line);
		_locked = hMutex->Lock();
		OutputDebugString("DSL_Mutex::Locked()\n");
		printf("DSL_Mutex::Locked(%s, %d)\n", fn.c_str(), line);
	}
	~DSL_MutexLocker() {
		OutputDebugString("DSL_Mutex::Release()\n");
		printf("DSL_Mutex::Release(%s, %d)\n", fn.c_str(), line);
		if (locked) {
			hMutex->Release();
		}
		printf("DSL_Mutex::Released(%s, %d)\n", fn.c_str(), line);
		OutputDebugString("DSL_Mutex::Released()\n");
	}
#else
	DSL_MutexLocker(DSL_Mutex * mutex) {
		hMutex = mutex;
		_locked = LockMutexPtr(mutex);
	}
	~DSL_MutexLocker() {
		if (locked) {
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
