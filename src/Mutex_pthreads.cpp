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

#if !defined(WIN32)

#include <drift/Mutex.h>
#include <drift/threading.h>
#include <assert.h>

DSL_Mutex_pthreads::DSL_Mutex_pthreads(int timeout) {
	refcnt = 0;
	lock_timeout = timeout;
	memset(&hMutex, 0, sizeof(hMutex));

	// I like my mutex'es recursive
	pthread_mutexattr_t pma;
	memset(&pma, 0, sizeof(pma));
	pthread_mutexattr_init(&pma);
	pthread_mutexattr_settype(&pma, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&hMutex, &pma);
	pthread_mutexattr_destroy(&pma);
}

DSL_Mutex_pthreads::~DSL_Mutex_pthreads() {
	pthread_mutex_destroy(&hMutex);
}

bool DSL_Mutex_pthreads::Lock() {
	if (lock_timeout >= 0) {
		return Lock(lock_timeout);
	}

	int n = pthread_mutex_lock(&hMutex);
	if (n != 0) {
		printf("Error locking mutex! (%p, %p, %d, %s)\n", this, &hMutex, n, strerror(n));
		assert(0);
	}
	LockingThreadID = pthread_self();
	refcnt++;
	if (refcnt <= 0) { refcnt = 1; }
	return true;
}

bool DSL_Mutex_pthreads::Lock(int timeo) {
	int64 timeout = timeo;
	struct timespec abs_time;
	memset(&abs_time, 0, sizeof(abs_time));
	if (clock_gettime(CLOCK_REALTIME, &abs_time) == 0) {
		abs_time.tv_sec += (timeout / 1000);
		timeout -= (timeout / 1000);
		abs_time.tv_nsec += timeout * 1000000;
		while (abs_time.tv_nsec >= 1000000000LL) {
			abs_time.tv_sec++;
			abs_time.tv_nsec -= 1000000000LL;
		}
		if (abs_time.tv_nsec < 0) {
			printf("Timed mutex lock nsec < 0! (%p, %p, %llu, %llu)\n", this, &hMutex, abs_time.tv_sec, abs_time.tv_nsec);
			abs_time.tv_nsec = 0;
		}
		int n = pthread_mutex_timedlock(&hMutex, &abs_time);
		if (n != 0) {
			if (timeout > 1000) {
				printf("Error locking mutex! (%p, %p, %d, %s)\n", this, &hMutex, n, strerror(n));
				printf("" I64FMT " / " I64FMT "\n", abs_time.tv_sec, abs_time.tv_nsec);
			}
			return false;
		}
	} else {
		//fallback in case clock_gettime fails
		int left = timeout;
		int amIn = pthread_mutex_trylock(&hMutex);
		while (amIn != 0) {
			if (left > 0) {
				safe_sleep(100, true);
				left -= 100;
				amIn = pthread_mutex_trylock(&hMutex);
			} else {
				if (timeout > 1000) {
					printf("Error locking mutex! (%p, %p, %s)\n", this, &hMutex, strerror(errno));
				}
				return false;
			}
		}
	}

	LockingThreadID = GetCurrentThreadId();
	refcnt++;
	if (refcnt <= 0) { refcnt = 1; }
	return true;
}

void DSL_Mutex_pthreads::Release() {
	refcnt--;
	if (refcnt <= 0) {
		LockingThreadID = 0;
	}
	int n = pthread_mutex_unlock(&hMutex);
	if (n != 0) {
		printf("Error unlocking mutex! (%p, %p, %d, %s)\n", this, &hMutex, n, strerror(n));
		assert(0);
	}
}

THREADIDTYPE DSL_Mutex_pthreads::LockingThread() {
	return LockingThreadID;
}

bool DSL_Mutex_pthreads::IsLockMine() {
	if (refcnt > 0 && pthread_equal(pthread_self(), LockingThreadID)) { return true; }
	return false;
}

bool DSL_Mutex_pthreads::IsLocked() {
	if (refcnt > 0) { return true; }
	return false;
}

#endif
