//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_BUFFER_H__
#define __DSL_BUFFER_H__

#include <drift/mutex.h>

/**
 * \defgroup buffer Data Buffer
 */

/** \addtogroup buffer
 * @{
 */

/** \struct DSL_BUFFER
 * Data Buffer structure. data/udata are just a convenience union to point to the same data as a signed or unsigned ptr.
 */
struct DSL_BUFFER {
	DSL_Mutex * hMutex; ///< Handle to the mutex protecting this buffer, if enabled.
	union {
		char * data;
		uint8 * udata;
	};
	int64 len;
	int64 capacity;
};

DSL_API void DSL_CC buffer_init(DSL_BUFFER * buf, bool useMutex = false); ///< Initialize the buffer, optionally with a mutex protecting it. If you don't use the mutex you need to either synchronize access yourself or only use it from a single thread.
DSL_API void DSL_CC buffer_free(DSL_BUFFER * buf); ///< Free the buffer when you are done with it.
DSL_API_CLASS string buffer_as_string(DSL_BUFFER * buf); ///< Gets the buffer as a string

DSL_API void DSL_CC buffer_clear(DSL_BUFFER * buf, bool force_free = false); ///< Sets the buffer length to 0 and clears the data. It is still ready to be used unlike buffer_free. By default the underlying memory isn't actually freed so it's ready to reuse without new allocations (same as STL vectors), set force_free = true to actually free it.
DSL_API void DSL_CC buffer_set(DSL_BUFFER * buf, const char * ptr, int64 len); ///< Sets the buffer to the specified data, discarding anything existing.
DSL_API void DSL_CC buffer_resize(DSL_BUFFER * buf, int64 len); ///< Resize the buffer, if the length is longer then the existing data the added byte values are undefined.

DSL_API void DSL_CC buffer_remove_front(DSL_BUFFER * buf, int64 len); ///< Remove data from the beginning of the buffer.
DSL_API void DSL_CC buffer_remove_end(DSL_BUFFER * buf, int64 len); ///< Remove data from the end of the buffer.
DSL_API bool DSL_CC buffer_prepend(DSL_BUFFER * buf, const char * ptr, int64 len); ///< Add data to the beginning of the buffer.
DSL_API bool DSL_CC buffer_append(DSL_BUFFER * buf, const char * ptr, int64 len); ///< Add data to the end of the buffer.

/**
 * Adds an int to the end of the buffer in native endianness.
 * Usage: Example to add a 32-bit unsigned int with value 12345: buffer_append_int<uint32>(&buf, 12345);
 */
template <typename T> bool buffer_append_int(DSL_BUFFER * buf, T y) { return buffer_append(buf, (char *)&y, sizeof(y)); }

/**@}*/

#endif // __DSL_BUFFER_H__
