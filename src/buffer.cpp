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
#include <drift/buffer.h>
#include <assert.h>

#pragma warning(disable: 4244)

#ifdef DSL_BUFFER_USE_VECTOR

void DSL_CC buffer_init(DSL_BUFFER * buf, bool useMutex) {
	memset(buf, 0, sizeof(DSL_BUFFER));
	buf->vec = new vector<uint8>;
	if (useMutex) { buf->hMutex = new DSL_Mutex(); }
}

void DSL_CC buffer_free(DSL_BUFFER * buf) {
	delete buf->vec;
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
	buf->vec->clear();
	buf->len = 0;
	buf->data = NULL;
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_set(DSL_BUFFER * buf, const char * ptr, int64 len) {
	if (buf->hMutex) { buf->hMutex->Lock(); }
	buf->vec->resize(len);
	memcpy(buf->vec->data(), ptr, len);
	buf->data = (char *)buf->vec->data();
	buf->len = len;
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_resize(DSL_BUFFER * buf, int64 len) {
	if (buf->hMutex) { buf->hMutex->Lock(); }
	buf->vec->resize(len);
	buf->data = (char *)buf->vec->data();
	buf->len = len;
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_remove_front(DSL_BUFFER * buf, int64 len) {
	if (len <= 0) { return; }
	if (buf->hMutex) { buf->hMutex->Lock(); }

	if (len >= buf->len) {
		buffer_clear(buf);
	} else {
		buf->vec->erase(buf->vec->begin(), buf->vec->begin() + len);
		buf->data = (char *)buf->vec->data();
		buf->len -= len;
	}
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_remove_end(DSL_BUFFER * buf, int64 len) {
	if (len <= 0) { return; }
	if (buf->hMutex) { buf->hMutex->Lock(); }
	if (len >= buf->len) {
		buffer_clear(buf);
	} else {
		buf->vec->resize(buf->vec->size() - len);
		buf->data = (char *)buf->vec->data();
		buf->len = buf->vec->size();
	}
	if (buf->hMutex) { buf->hMutex->Release(); }
}

bool DSL_CC buffer_prepend(DSL_BUFFER * buf, const char * ptr, int64 len) {
	if (len <= 0) { return true; }
	if (buf->hMutex) { buf->hMutex->Lock(); }

#if 0
	buf->vec->insert(buf->vec->begin(), ptr, ptr + len);
	buf->data = (char *)buf->vec->data();
	buf->len += len;
#else
	buf->vec->resize(buf->len + len);
	buf->data = (char *)buf->vec->data();
	memmove(buf->data + len, buf->data, buf->len);
	memcpy(buf->data, ptr, len);
	buf->len += len;
#endif

	if (buf->hMutex) { buf->hMutex->Release(); }
	return true;
}

bool DSL_CC buffer_append(DSL_BUFFER * buf, const char * ptr, int64 len) {
	if (len <= 0) { return true; }
	if (buf->hMutex) { buf->hMutex->Lock(); }

	buf->vec->resize(buf->len + len);
	assert(buf->vec->size() == buf->len + len);
	buf->data = (char *)buf->vec->data();
	memcpy(buf->data + buf->len, ptr, len);
	buf->len += len;

	if (buf->hMutex) { buf->hMutex->Release(); }
	return true;
}

#else

static void buffer_grow(DSL_BUFFER * buf, int64 needed) {
	if (needed <= buf->capacity) { return; }
	int64 newcap = buf->capacity ? buf->capacity : 64;
	while (newcap < needed) { newcap *= 2; }
	buf->data = (char *)dsl_realloc(buf->data, newcap);
	buf->capacity = newcap;
}

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
	buf->capacity = 0;
	buf->data = NULL;
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_set(DSL_BUFFER * buf, const char * ptr, int64 len) {
	if (buf->hMutex) { buf->hMutex->Lock(); }
	buffer_grow(buf, len);
	memcpy(buf->data, ptr, len);
	buf->len = len;
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_resize(DSL_BUFFER * buf, int64 len) {
	if (buf->hMutex) { buf->hMutex->Lock(); }
	buffer_grow(buf, len);
	buf->len = len;
	if (buf->hMutex) { buf->hMutex->Release(); }
}

void DSL_CC buffer_remove_front(DSL_BUFFER * buf, int64 len) {
	if (len <= 0) { return; }
	if (buf->hMutex) { buf->hMutex->Lock(); }
	if (len >= buf->len) {
		buf->len = 0;
	} else {
		buf->len -= len;
		memmove(buf->data, buf->data + len, buf->len);
	}
	if (buf->hMutex) { buf->hMutex->Release(); }
}
void DSL_CC buffer_remove_end(DSL_BUFFER * buf, int64 len) {
	if (len <= 0) { return; }
	if (buf->hMutex) { buf->hMutex->Lock(); }
	if (len >= buf->len) {
		buf->len = 0;
	} else {
		buf->len -= len;
	}
	if (buf->hMutex) { buf->hMutex->Release(); }
}
bool DSL_CC buffer_prepend(DSL_BUFFER * buf, const char * ptr, int64 len) {
	if (len <= 0) { return true; }
	if (buf->hMutex) { buf->hMutex->Lock(); }

	buffer_grow(buf, buf->len + len);
	memmove(buf->data + len, buf->data, buf->len);
	memcpy(buf->data, ptr, len);
	buf->len += len;

	if (buf->hMutex) { buf->hMutex->Release(); }
	return true;
}
bool DSL_CC buffer_append(DSL_BUFFER * buf, const char * ptr, int64 len) {
	if (len <= 0) { return true; }
	if (buf->hMutex) { buf->hMutex->Lock(); }

	buffer_grow(buf, buf->len + len);
	memcpy(buf->data + buf->len, ptr, len);
	buf->len += len;

	if (buf->hMutex) { buf->hMutex->Release(); }
	return true;
}

#endif
