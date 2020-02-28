//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef _INCLUDE_MUTEX_H_
#define _INCLUDE_MUTEX_H_

#include <drift/Threading.h>

#if !defined(DSL_DEFAULT_MUTEX_TIMEOUT)
#define DSL_DEFAULT_MUTEX_TIMEOUT -1
#endif

class DSL_API_CLASS Titus_Mutex_Base {
public:
	virtual bool Lock(int timeout)=0;
	virtual bool Lock()=0;
	virtual void Release()=0;

	virtual void SetLockTimeout(int timeout)=0;
	virtual bool IsLocked()=0;
	virtual bool IsLockMine()=0;

	virtual THREADIDTYPE LockingThread()=0;
};

class DSL_API_CLASS Titus_MutexLocker {
private:
	Titus_Mutex_Base * hMutex;
public:
	Titus_MutexLocker(Titus_Mutex_Base * mutex);
	~Titus_MutexLocker();
};

#if defined(WIN32) || defined(XBOX)

class DSL_API_CLASS Titus_Mutex_Win32CS : public Titus_Mutex_Base {
	private:
		THREADIDTYPE LockingThreadID;
		int refcnt;
		int lock_timeout;
		CRITICAL_SECTION cs;
	public:
		Titus_Mutex_Win32CS(int timeout = DSL_DEFAULT_MUTEX_TIMEOUT);
		~Titus_Mutex_Win32CS();

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

class DSL_API_CLASS Titus_Mutex_Win32Mutex : public Titus_Mutex_Base {
	private:
		THREADIDTYPE LockingThreadID;
		int refcnt;
		int lock_timeout;
		HANDLE hMutex;
	public:
		Titus_Mutex_Win32Mutex(int timeout = DSL_DEFAULT_MUTEX_TIMEOUT, const char * name = NULL);
		~Titus_Mutex_Win32Mutex();

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
#define Titus_Mutex Titus_Mutex_Win32Mutex
#define Titus_TimedMutex Titus_Mutex_Win32Mutex
#else
#define Titus_Mutex Titus_Mutex_Win32CS
#define Titus_TimedMutex Titus_Mutex_Win32Mutex
#endif

#else // defined(WIN32) || defined(XBOX)

class DSL_API_CLASS Titus_Mutex_pthreads : public Titus_Mutex_Base {
	private:
		THREADIDTYPE LockingThreadID;
		int refcnt;
		int lock_timeout;
		pthread_mutex_t hMutex;
	public:
		Titus_Mutex_pthreads(int timeout = DSL_DEFAULT_MUTEX_TIMEOUT);
		virtual ~Titus_Mutex_pthreads();

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

#define Titus_Mutex Titus_Mutex_pthreads
#define Titus_TimedMutex Titus_Mutex_pthreads
#endif // defined(WIN32) || defined(XBOX)

#ifdef DEBUG_MUTEX
#define LockMutex(x) { \
	OutputDebugString("Titus_Mutex::Lock()\n"); \
	printf("Titus_Mutex::Lock(%s, %d)\n", __FILE__, __LINE__); \
	x.Lock(); \
	OutputDebugString("Titus_Mutex::Locked()\n"); \
	printf("Titus_Mutex::Locked(%s, %d)\n", __FILE__, __LINE__); \
}
#define LockMutexPtr(x) { \
	OutputDebugString("Titus_Mutex::Lock()\n"); \
	printf("Titus_Mutex::Lock(%s, %d)\n", __FILE__, __LINE__); \
	x->Lock(); \
	OutputDebugString("Titus_Mutex::Locked()\n"); \
	printf("Titus_Mutex::Locked(%s, %d)\n", __FILE__, __LINE__); \
}
#define RelMutex(x) { printf("Titus_Mutex::Release(%s, %d)\n", __FILE__, __LINE__); x.Release(); }
#define RelMutexPtr(x) { OutputDebugString("Titus_Mutex::Release()\n"); printf("Titus_Mutex::Release(%s, %d)\n", __FILE__, __LINE__); x->Release(); printf("Titus_Mutex::Released(%s, %d)\n", __FILE__, __LINE__); OutputDebugString("Titus_Mutex::Released()\n");  }
#else
#define LockMutex(x) x.Lock()
#define LockMutexPtr(x) x->Lock()
#define TryLockMutex(x, y) x.Lock(y)
#define TryLockMutexPtr(x, y) x->Lock(y)
#define RelMutex(x) x.Release()
#define RelMutexPtr(x) x->Release()
#endif

#define AutoMutex(x) Titus_MutexLocker MAKE_UNIQUE_NAME (&x)
#define AutoMutexPtr(x) Titus_MutexLocker MAKE_UNIQUE_NAME (x)

// *** DO NOT USE THIS, FOR DSL INTERNAL USE ONLY ***
Titus_Mutex * dslMutex();

#endif // _INCLUDE_MUTEX_H_
