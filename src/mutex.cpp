//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2025 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dslcore.h>
#include <drift/mutex.h>
#include <drift/threading.h>
#include <assert.h>

DSL_Mutex::DSL_Mutex(int timeout) {
	lock_timeout = timeout;
}

bool DSL_Mutex::Lock() {
	if (lock_timeout >= 0) {
		return Lock(lock_timeout);
	}

	hMutex.lock();
	return true;
}

bool DSL_Mutex::Lock(int timeout) {
	if (timeout < 0) {
		hMutex.lock();
		return true;
	} else if (timeout == 0) {
		return hMutex.try_lock();
	} else {
		chrono::milliseconds wait_time(timeout);
		return hMutex.try_lock_for(wait_time);
	}
}

void DSL_Mutex::Release() {
	hMutex.unlock();
}
