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
#include <drift/Mutex.h>
#include <drift/Threading.h>

/*
#if defined(WIN32)
Titus_MutexLocker::Titus_MutexLocker(Titus_Mutex_Win32Mutex * mutex, const char * fn, int line) {
	hMutexCS = NULL;
	hMutex = mutex;
	LockMutexPtr(mutex);
}
Titus_MutexLocker::Titus_MutexLocker(Titus_Mutex_Win32CS * mutex, const char * fn, int line) {
	hMutex = NULL;
	hMutexCS = mutex;
	LockMutexPtr(mutex);
}
Titus_MutexLocker::~Titus_MutexLocker() {
	if (hMutex != NULL) {
		RelMutexPtr(hMutex);
	}
	if (hMutexCS != NULL) {
		RelMutexPtr(hMutexCS);
	}
}
#else
*/
Titus_MutexLocker::Titus_MutexLocker(Titus_Mutex_Base * mutex) {
	hMutex = mutex;
	LockMutexPtr(mutex);
}
Titus_MutexLocker::~Titus_MutexLocker() {
	RelMutexPtr(hMutex);
}
//#endif

/*
Titus_Mutex hMutexWatchdog;
bool bMutexWatchdogShutdown = false;
bool bMutexWatchdogRunning = false;
map<Titus_Mutex *, time_t> watchdog_list;

TT_DEFINE_THREAD(Mutex_Watchdog) {
	TT_THREAD_START
	bMutexWatchdogRunning = true;
	while (!bMutexWatchdogShutdown) {
		hMutexWatchdog.Lock();
		time_t tme = time(NULL);
		for (auto x = watchdog_list.begin(); x != watchdog_list.begin(); x++) {
			Titus_Mutex * hMutex = x->first;
			if (hMutex->IsLocked()) {
				time_t exp = hMutex->last_mod_time + x->second;
				if (tme > exp) {
					printf("DSL Watchdog: Unlocking mutex %p\n", hMutex);
					hMutex->Release();
				}
			}
		}
		hMutexWatchdog.Release();

		safe_sleep(100, true);
	}
	bMutexWatchdogRunning = false;
	TT_THREAD_END
}

bool DSL_CC dsl_mutex_watchdog_init();
void DSL_CC dsl_mutex_watchdog_add(Titus_Mutex * hMutex, time_t timeout);
void DSL_CC dsl_mutex_watchdog_remove(Titus_Mutex * hMutex);
void DSL_CC dsl_mutex_watchdog_quit();

bool DSL_CC dsl_mutex_watchdog_init() {
	if (TT_StartThread(Mutex_Watchdog, NULL, "Mutex Watchdog") != NULL) {
		bMutexWatchdogRunning = true;
	}
}
void DSL_CC dsl_mutex_watchdog_add(Titus_Mutex * hMutex, time_t timeout) {
	AutoMutex(hMutexWatchdog);
	watchdog_list[hMutex] = timeout;
}
void DSL_CC dsl_mutex_watchdog_remove(Titus_Mutex * hMutex) {
	AutoMutex(hMutexWatchdog);
	auto x = watchdog_list.find(hMutex);
	if (x != watchdog_list.end()) {
		watchdog_list.erase(x);
	}
}
void DSL_CC dsl_mutex_watchdog_quit() {
	bMutexWatchdogShutdown = true;
	while (bMutexWatchdogRunning) {
		safe_sleep(100, true);
	}
}
*/
