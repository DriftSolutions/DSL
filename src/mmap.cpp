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
#include <drift/mmap.h>
#include <assert.h>
#ifndef WIN32
#include <sys/mman.h>
#endif

size_t DSL_CC dsl_map_page_size() {
	static size_t page_size = 0;
	if (page_size) {
		return page_size;
	}
#ifdef WIN32
	SYSTEM_INFO si;
	memset(&si, 0, sizeof(si));
	GetSystemInfo(&si);
	page_size = si.dwAllocationGranularity;
#else
	page_size = sysconf(_SC_PAGE_SIZE);
#endif
	return page_size;
}

DSL_MMAP_HANDLE * DSL_CC dsl_map_handle(DSL_MMAP_OS_HANDLE fd, int64 size, int64 offset, uint8 flags) {
	bool fWrite = (flags & DSL_MMAP_WRITE) != 0;
	bool fExec = (flags & DSL_MMAP_EXEC) != 0;
#ifdef WIN32
	LARGE_INTEGER li;
	if (size == 0) {
		if (!GetFileSizeEx(fd, &li)) {
			return NULL;
		}
		size = li.QuadPart - offset;
	}
	if (size <= 0 || offset < 0 || offset >= size) {
		return NULL;
	}
	if (sizeof(SIZE_T) < 8 && size > UINT32_MAX) {
		return NULL;
	}

	li.QuadPart = size;
	DWORD mFlags = 0;
	if (fExec) {
		if (fWrite) {
			mFlags = PAGE_EXECUTE_READWRITE;
		} else {
			mFlags = PAGE_EXECUTE_READ;
		}
	} else {
		if (fWrite) {
			mFlags = PAGE_READWRITE;
		} else {
			mFlags = PAGE_READONLY;
		}
	}
	HANDLE hMap = CreateFileMapping(fd, NULL, mFlags, li.HighPart, li.LowPart, NULL);
	if (hMap == NULL) {
		return NULL;
	}

	DWORD mAccess = fWrite ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ;
	if (fExec) {
		mAccess |= FILE_MAP_EXECUTE;
	}
	li.QuadPart = offset;
	void * data = MapViewOfFile(hMap, mAccess, li.HighPart, li.LowPart, size);
	if (data == NULL) {
		CloseHandle(hMap);
		return NULL;
	}

	DSL_MMAP_HANDLE * ret = dsl_new(DSL_MMAP_HANDLE);
	ret->flags = flags;
	ret->hFile = fd;
	ret->hMap = hMap;
	ret->data = data;
	ret->size = size;
	return ret;	
#else
	if (size == 0) {
		size = lseek64(fd, 0, SEEK_END);
		lseek64(fd, 0, SEEK_SET);
	}
	if (size <= 0 || offset < 0 || offset >= size) {
		return NULL;
	}
	if (sizeof(size_t) < 8 && size > UINT32_MAX) {
		return NULL;
	}
	if (sizeof(off_t) < 8 && offset > UINT32_MAX) {
		return NULL;
	}

	int prot = PROT_READ;
	if (fWrite) {
		prot |= PROT_WRITE;
	}
	if (fExec) {
		prot |= PROT_EXEC;
	}
	void * data = mmap(NULL, size, prot, MAP_SHARED, fd, offset);
	if (data == MAP_FAILED) {
		return NULL;
	}

	DSL_MMAP_HANDLE * ret = dsl_new(DSL_MMAP_HANDLE);
	ret->flags = flags;
	ret->fd = fd;
	ret->data = data;
	ret->size = size;
	return ret;
#endif

}

DSL_MMAP_HANDLE * DSL_CC dsl_map_file(const char * fn, int64 size, int64 offset, uint8 flags) {
	bool fWrite = (flags & DSL_MMAP_WRITE) != 0;
#ifdef WIN32
	if (sizeof(SIZE_T) < 8 && size > UINT32_MAX) {
		return NULL;
	}
	DWORD wModes = GENERIC_READ;
	if (fWrite) {
		wModes |= GENERIC_WRITE;
	}
	HANDLE hFile = CreateFile(fn, wModes, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return NULL;
	}
	DSL_MMAP_HANDLE * ret = dsl_map_handle(hFile, size, offset, flags | DSL_MMAP_CLOSE);
	if (ret == NULL) {
		CloseHandle(hFile);
	}
	return ret;
#else
	int oflags = fWrite ? O_RDWR : O_RDONLY;
	int  fd = open(fn, oflags | O_CREAT);
	if (fd == -1) {
		return NULL;
	}
	DSL_MMAP_HANDLE * ret = dsl_map_handle(fd, size, offset, flags | DSL_MMAP_CLOSE);
	if (ret == NULL) {
		close(fd);
	}
	return ret;
#endif
}

void DSL_CC dsl_unmap_file(DSL_MMAP_HANDLE * h) {
#ifdef WIN32
	UnmapViewOfFile(h->data);
	CloseHandle(h->hMap);
	if (h->flags & DSL_MMAP_CLOSE) {
		CloseHandle(h->hFile);
	}
#else
	munmap(h->data, h->size);
	close(h->fd);
#endif
	dsl_free(h);
}
