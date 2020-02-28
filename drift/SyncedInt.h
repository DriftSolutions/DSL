//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef _INCLUDE_SYNCEDINT_H_
#define _INCLUDE_SYNCEDINT_H_

#include <drift/Mutex.h>

template <class T>
class SyncedInt {
#if defined(WIN32)
private:
	T * value;
public:
	SyncedInt() {
		value = (T *)_aligned_malloc(sizeof(T), 64);
		*value = 0;
	}
	~SyncedInt() {
		_aligned_free(value);
	}
	T Increment() {
		if (sizeof(T) == 4) {
			return InterlockedIncrement((LONG *)value);
#if (_WIN32_WINNT >= 0x0600)
		} else if (sizeof(T) == 8) {
			return InterlockedIncrement64((LONGLONG *)value);
#endif
#if (_WIN32_WINNT >= 0x0602)
		} else if (sizeof(T) == 2) {
			return InterlockedIncrement16((LONGLONG *)value);
#endif
		} else {
			printf("SyncedInt: Unsupported int size: %u\n", sizeof(T));
		}
		return -1;
	}
	T Decrement() {
		if (sizeof(T) == 4) {
			return InterlockedDecrement((LONG *)value);
#if (_WIN32_WINNT >= 0x0600)
		} else if (sizeof(T) == 8) {
			return InterlockedDecrement64((LONGLONG *)value);
#endif
#if (_WIN32_WINNT >= 0x0602)
		} else if (sizeof(T) == 2) {
			return InterlockedDecrement16((LONGLONG *)value);
#endif
		} else {
			printf("SyncedInt: Unsupported int size: %u\n", sizeof(T));
		}
		return -1;
	}
	T Get() { return *value; }
#else
private:
	T __attribute__((aligned(64))) value;
public:
	SyncedInt() {
		value = 0;
	}
	T Increment() {
		return __sync_add_and_fetch(&value, 1);
	}
	T Decrement() {
		return __sync_sub_and_fetch(&value, 1);
	}
	T Get() { return value; }
#endif
};

#endif // _INCLUDE_SYNCEDINT_H_
