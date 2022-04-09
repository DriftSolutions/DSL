//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dslcore.h>
#include <drift/Buffer.h>

#pragma warning(disable: 4244)

void DSL_CC buffer_init(DSL_BUFFER * buf, bool useMutex) {
	memset(buf, 0, sizeof(DSL_BUFFER));
	if (useMutex) { buf->hMutex = new DSL_Mutex(); }
}

void DSL_CC buffer_free(DSL_BUFFER * buf) {
	dsl_freenn(buf->data);
	if (buf->hMutex) { delete buf->hMutex; }
	memset(buf, 0xFE, sizeof(DSL_BUFFER));
}

string buffer_as_string(DSL_BUFFER * buf) {
	string ret;
	if (buf->hMutex) { buf->hMutex->Lock(); }
	if (buf->len > 0) {
		ret.assign(buf->data, buf->len);
	}
	if (buf->hMutex) { buf->hMutex->Release(); }
	return ret;
}

void DSL_CC buffer_clear(DSL_BUFFER * buf) {
	if (buf->hMutex) { buf->hMutex->Lock(); }
	dsl_freenn(buf->data);
	buf->len = 0;
	buf->data = NULL;
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_set(DSL_BUFFER * buf, const char * ptr, int64 len) {
	if (buf->hMutex) { buf->hMutex->Lock(); }
	buf->data = (char *)dsl_realloc(buf->data, len);
	buf->len = len;
	memcpy(buf->data, ptr, len);
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_resize(DSL_BUFFER * buf, int64 len) {
	if (buf->hMutex) { buf->hMutex->Lock(); }
	buf->data = (char *)dsl_realloc(buf->data, len);
	buf->len = len;
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_remove_front(DSL_BUFFER * buf, int64 len) {
	if (len <= 0) {	return;	}
	if (buf->hMutex) { buf->hMutex->Lock(); }
	if (len >= buf->len) {
		buffer_clear(buf);
	} else {
		buf->len -= len;
		memmove(buf->data, buf->data + len, buf->len);
		buf->data = (char *)dsl_realloc(buf->data, buf->len);
	}
	if (buf->hMutex) { buf->hMutex->Release(); }
}
void DSL_CC buffer_remove_end(DSL_BUFFER * buf, int64 len) {
	if (len <= 0) { return; }
	if (buf->hMutex) { buf->hMutex->Lock(); }
	if (len >= buf->len) {
		buffer_clear(buf);
	} else {
		buf->len -= len;
		buf->data = (char *)dsl_realloc(buf->data, buf->len);
	}
	if (buf->hMutex) { buf->hMutex->Release(); }
}
bool DSL_CC buffer_prepend(DSL_BUFFER * buf, const char * ptr, int64 len) {
	if (len <= 0) { return true; }
	if (buf->hMutex) { buf->hMutex->Lock(); }

	buf->data = (char *)dsl_realloc(buf->data, buf->len + len);
	memmove(buf->data + len, buf->data, buf->len);
	memcpy(buf->data, ptr, len);
	buf->len += len;

	if (buf->hMutex) { buf->hMutex->Release(); }
	return true;
}
bool DSL_CC buffer_append(DSL_BUFFER * buf, const char * ptr, int64 len) {
	if (len <= 0) { return true; }
	if (buf->hMutex) { buf->hMutex->Lock(); }

	buf->data = (char *)dsl_realloc(buf->data, buf->len + len);
	memcpy(buf->data + buf->len, ptr, len);
	buf->len += len;

	if (buf->hMutex) { buf->hMutex->Release(); }
	return true;
}

DSL_Buffer::DSL_Buffer() {
	data = NULL;
	len  = 0;
}

DSL_Buffer::~DSL_Buffer() { Clear(); }

void DSL_Buffer::Clear() {
	LockMutex(hMutex);
	if (data) { dsl_free(data); }
	data = NULL;
	len  = 0;
	RelMutex(hMutex);
}
void DSL_Buffer::RemoveFromEnd(uint32 ulen) {
	if (ulen >= len) {
		Clear();
	} else {
		LockMutex(hMutex);
		len -= ulen;
		data = (char *)dsl_realloc(data, len);
		RelMutex(hMutex);
	}
}

void DSL_Buffer::RemoveFromBeginning(uint32 ulen) {
	if (ulen >= len) {
		Clear();
	} else {
		LockMutex(hMutex);
		len -= ulen;
		memmove(data, data+ulen, len);
		data = (char *)dsl_realloc(data, len);
		RelMutex(hMutex);
	}
}

char * DSL_Buffer::Get() {
	return data;
}

uint32 DSL_Buffer::GetLen() {
	return len;
}

bool DSL_Buffer::Get(char * buf, uint32 * size) {
	LockMutex(hMutex);
	*size = len;
	if (*size < len) {
		memcpy(buf, data, *size);
		RelMutex(hMutex);
		return false;
	}
	memcpy(buf, data, len);
	RelMutex(hMutex);
	return false;
}

bool DSL_Buffer::Get(char * buf, uint32 size) {
	if (data == NULL) { return size > 0 ? false:true; }
	LockMutex(hMutex);
	if (size < len) {
		memcpy(buf, data, size);
		RelMutex(hMutex);
		return false;
	}
	memcpy(buf, data, len);
	RelMutex(hMutex);
	return false;
}

void DSL_Buffer::Set(const char * buf, uint32 ulen) {
	LockMutex(hMutex);
	if (ulen == 0xFFFFFFFF) { ulen = uint32(strlen(buf)); }
	data = (char *)dsl_realloc(data, ulen);
	memcpy(data, buf, ulen);
	len = ulen;
	RelMutex(hMutex);
}
void DSL_Buffer::Append(const char * buf, uint32 ulen) {
	LockMutex(hMutex);
	if (ulen == 0xFFFFFFFF) { ulen = uint32(strlen(buf)); }
	data = (char *)dsl_realloc(data, len+ulen);
	memcpy(data+len, buf, ulen);
	len += ulen;
	RelMutex(hMutex);
}
void DSL_Buffer::Prepend(const char * buf, uint32 ulen) {
	LockMutex(hMutex);
	if (ulen == 0xFFFFFFFF) { ulen = uint32(strlen(buf)); }
	data = (char *)dsl_realloc(data, len+ulen);
	memmove(data+ulen, data, len);
	memcpy(data, buf, ulen);
	len += ulen;
	RelMutex(hMutex);
};

void DSL_Buffer::Append_int8(int8 val) {
	Append((char *)&val, sizeof(val));
}
void DSL_Buffer::Append_uint8(uint8 val) {
	Append((char *)&val, sizeof(val));
}
void DSL_Buffer::Append_int16(int16 val) {
	Append((char *)&val, sizeof(val));
}
void DSL_Buffer::Append_uint16(uint16 val) {
	Append((char *)&val, sizeof(val));
}
void DSL_Buffer::Append_int32(int32 val) {
	Append((char *)&val, sizeof(val));
}
void DSL_Buffer::Append_uint32(uint32 val) {
	Append((char *)&val, sizeof(val));
}
void DSL_Buffer::Append_int64(int64 val) {
	Append((char *)&val, sizeof(val));
}
void DSL_Buffer::Append_uint64(uint64 val) {
	Append((char *)&val, sizeof(val));
}
