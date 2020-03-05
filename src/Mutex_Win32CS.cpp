//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#include <drift/dslcore.h>

#if defined(WIN32)

#include <drift/Mutex.h>
#include <drift/Threading.h>
#include <assert.h>

DSL_Mutex_Win32CS::DSL_Mutex_Win32CS(int timeout) {
	refcnt = 0;
	LockingThreadID = 0;
	lock_timeout = timeout;
	memset(&cs, 0, sizeof(cs));
	InitializeCriticalSection(&cs);
}

DSL_Mutex_Win32CS::~DSL_Mutex_Win32CS() {
	assert(refcnt <= 0);
	DeleteCriticalSection(&cs);
}

bool DSL_Mutex_Win32CS::Lock() {
	if (lock_timeout >= 0) {
		return Lock(lock_timeout);
	}

	EnterCriticalSection(&cs);
	LockingThreadID = GetCurrentThreadId();
	refcnt++;
	if (refcnt <= 0) { refcnt = 1; }
	return true;
}

bool DSL_Mutex_Win32CS::Lock(int timeout) {
	int left = timeout;
	BOOL amIn = TryEnterCriticalSection(&cs);
	while (!amIn) {
		if (left > 0) {
			safe_sleep(100, true);
			left -= 100;
			amIn = TryEnterCriticalSection(&cs);
		} else {
			if (timeout > 1000) {
				printf("Timeout while locking mutex! (%p, %p, %d)\n", this, &cs, GetLastError());
			}
			return false;
		}
	}
	LockingThreadID = GetCurrentThreadId();
	refcnt++;
	if (refcnt <= 0) { refcnt = 1; }
	return true;
}

void DSL_Mutex_Win32CS::Release() {
	assert(refcnt > 0);
	refcnt--;
	if (refcnt <= 0) {
		LockingThreadID = 0;
	}
	LeaveCriticalSection(&cs);
}

THREADIDTYPE DSL_Mutex_Win32CS::LockingThread() {
	return LockingThreadID;
}

bool DSL_Mutex_Win32CS::IsLockMine() {
	if (refcnt > 0 && GetCurrentThreadId() == LockingThreadID) { return true; }
	return false;
}

bool DSL_Mutex_Win32CS::IsLocked() {
	if (refcnt > 0) { return true; }
	return false;
}

#endif
