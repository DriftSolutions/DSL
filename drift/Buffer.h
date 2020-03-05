//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef __DSL_BUFFER_H__
#define __DSL_BUFFER_H__

#include <drift/Mutex.h>

struct DSL_BUFFER {
	DSL_Mutex * hMutex;
	union {
		char * data;
		uint8 * udata;
	};
	int64 len;
};

DSL_API void DSL_CC buffer_init(DSL_BUFFER * buf, bool useMutex = false);
DSL_API void DSL_CC buffer_free(DSL_BUFFER * buf);

DSL_API void DSL_CC buffer_clear(DSL_BUFFER * buf);
DSL_API void DSL_CC buffer_set(DSL_BUFFER * buf, const char * ptr, int64 len);
DSL_API void DSL_CC buffer_resize(DSL_BUFFER * buf, int64 len);

DSL_API void DSL_CC buffer_remove_front(DSL_BUFFER * buf, int64 len);
DSL_API void DSL_CC buffer_remove_end(DSL_BUFFER * buf, int64 len);
DSL_API bool DSL_CC buffer_prepend(DSL_BUFFER * buf, const char * ptr, int64 len);
DSL_API bool DSL_CC buffer_append(DSL_BUFFER * buf, const char * ptr, int64 len);

//#define buffer_append_uint8(x,y) buffer_append(x, &y, 1)
template <typename T> bool buffer_append_int(DSL_BUFFER * buf, T y) { return buffer_append(buf, (char *)&y, sizeof(y)); }

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
