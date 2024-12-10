
#ifndef __DSL_MMAP_H__
#define __DSL_MMAP_H__

//Pages may be executed.
#define DSL_MMAP_EXEC 0x01
//Pages may be read.
#define DSL_MMAP_READ 0x02
//Pages may be written.
#define DSL_MMAP_WRITE 0x04
//Close file handle on unmap
#define DSL_MMAP_CLOSE 0x08

#ifdef WIN32
#define DSL_MMAP_OS_HANDLE HANDLE
#else
#define DSL_MMAP_OS_HANDLE int
#endif

struct DSL_MMAP_HANDLE{
	void * data;
	int64 size;
	uint8 flags;
#ifdef WIN32
	HANDLE hFile, hMap;
#else
	int fd;
	void * mmap_ptr;
#endif
};

/*
* @param size The size of the mapping, 0 = the size of the file or to the end of the file if offset > 0
* @param offset The offset in the file to map from, must be a multiple of the page size (see dsl_map_page_size())
*/
DSL_API DSL_MMAP_HANDLE * DSL_CC dsl_map_file(const char * fn, int64 size, uint64 offset = 0, uint8 flags = DSL_MMAP_READ);
DSL_API DSL_MMAP_HANDLE * DSL_CC dsl_map_handle(DSL_MMAP_OS_HANDLE fd, int64 size, uint64 offset = 0, uint8 flags = DSL_MMAP_READ);
DSL_API void DSL_CC dsl_unmap_file(DSL_MMAP_HANDLE *);
DSL_API size_t DSL_CC dsl_map_page_size();

/*
Under Win32 don't forget to wrap your data accesses in an exception handler like this:

__try {
	// access mapped data here
} __except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
}
*/

#endif // __DSL_MMAP_H__
