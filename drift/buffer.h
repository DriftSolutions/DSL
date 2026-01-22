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

//#define DSL_BUFFER_USE_VECTOR

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
#ifdef DSL_BUFFER_USE_VECTOR
	vector<uint8> * vec;
#else
	int64 capacity;
#endif
};

DSL_API void DSL_CC buffer_init(DSL_BUFFER * buf, bool useMutex = false); ///< Initialize the buffer, optionally with a mutex protecting it. If you don't use the mutex you need to either synchronize access yourself or only use it from a single thread.
DSL_API void DSL_CC buffer_free(DSL_BUFFER * buf); ///< Free the buffer when you are done with it.
DSL_API_CLASS string buffer_as_string(DSL_BUFFER * buf); ///< Gets the buffer as a string

DSL_API void DSL_CC buffer_clear(DSL_BUFFER * buf); ///< Sets the buffer length to 0 and clears the data. It is still ready to be used unlike buffer_free.
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

class DSL_API_CLASS DSL_Buffer {
private:
	DSL_Mutex hMutex;
	char * data;
	uint32 len;
public:
	DSL_Buffer();
	~DSL_Buffer();

	void Clear();
	void RemoveFromEnd(uint32 len);
	void RemoveFromBeginning(uint32 len);
	char * Get();
	uint32 GetLen();
	bool Get(char * buf, uint32 * size);//the call will set size to the current length of data
										//and will return true if the buffer was big enough and the copy was successful
										//otherwise will return false and copy up to size into your buffer
	bool Get(char * buf, uint32 size);	//the call will fill the buffer with your data up to size
										//and will return true if the buffer was big enough and the copy was successful
										//otherwise will return false and copy up to size into your buffer

	void Set(const char * buf, uint32 len=0xFFFFFFFF);
	void Append(const char * buf, uint32 len=0xFFFFFFFF);
	void Prepend(const char * buf, uint32 len=0xFFFFFFFF);

	void Append_int8(int8 val);
	void Append_uint8(uint8 val);
	void Append_int16(int16 val);
	void Append_uint16(uint16 val);
	void Append_int32(int32 val);
	void Append_uint32(uint32 val);
	void Append_int64(int64 val);
	void Append_uint64(uint64 val);
};

#endif // __DSL_BUFFER_H__
