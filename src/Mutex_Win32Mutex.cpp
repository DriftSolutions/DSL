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

#if defined(WIN32)

#include <drift/Mutex.h>
#include <drift/threading.h>
#include <assert.h>

DSL_Mutex_Win32Mutex::DSL_Mutex_Win32Mutex(int timeout, const char * name) {
	refcnt = 0;
	LockingThreadID = 0;
	lock_timeout = timeout;
	hMutex = CreateMutexA(NULL, FALSE, name);
}

DSL_Mutex_Win32Mutex::~DSL_Mutex_Win32Mutex() {
	CloseHandle(hMutex);
}

bool DSL_Mutex_Win32Mutex::Lock() {
	if (lock_timeout >= 0) {
		return Lock(lock_timeout);
	}

	DWORD ret = WaitForSingleObject(hMutex, INFINITE);
	if (ret != WAIT_OBJECT_0) {
		printf("Error while locking mutex! (%p, %p, %d, %d)\n", this, hMutex, ret, GetLastError());
		assert(0);
	}
	LockingThreadID = GetCurrentThreadId();
	refcnt++;
	if (refcnt <= 0) { refcnt = 1; }
	return true;
}

bool DSL_Mutex_Win32Mutex::Lock(int timeout) {
	DWORD ret = WaitForSingleObject(hMutex, timeout);
	if (ret == WAIT_TIMEOUT) {
		if (timeout > 1000) {
			printf("Timeout while locking mutex! (%p, %p, %d)\n", this, hMutex, GetLastError());
		}
		return false;
	}
	if (ret != WAIT_OBJECT_0) {
		printf("Error while locking mutex! (%p, %p, %d, %d)\n", this, hMutex, ret, GetLastError());
		assert(0);
		return false;
	}
	LockingThreadID = GetCurrentThreadId();
	refcnt++;
	if (refcnt <= 0) { refcnt = 1; }
	return true;
}

void DSL_Mutex_Win32Mutex::Release() {
	refcnt--;
	if (refcnt <= 0) {
		LockingThreadID = 0;
	}
	if (ReleaseMutex(hMutex) == 0) {
		printf("Error while unlocking mutex! (%p, %p, %d)\n", this, hMutex, GetLastError());
		assert(0);
	}
}

THREADIDTYPE DSL_Mutex_Win32Mutex::LockingThread() {
	return LockingThreadID;
}

bool DSL_Mutex_Win32Mutex::IsLockMine() {
	if (refcnt > 0 && GetCurrentThreadId() == LockingThreadID) { return true; }
	return false;
}

bool DSL_Mutex_Win32Mutex::IsLocked() {
	if (refcnt > 0) { return true; }
	return false;
}

#endif
