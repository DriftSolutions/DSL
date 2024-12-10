//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_DIRECTORY_H__
#define __DSL_DIRECTORY_H__

#ifdef __GLIBC__
	#if __GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 24)
		#warning "Old glibc, using readdir_r"
		#define __DSL_USE_READDIR_R__ 1
	#endif
#endif

/**
 * \defgroup directory Directory Reader
 */

/** \addtogroup directory
 * @{
 */

class DSL_API_CLASS Directory {
private:
	union {
		char * mDir;
		wchar_t * wDir;
	};
	#if defined(WIN32)
		HANDLE hFind;
		union {
			WIN32_FIND_DATAA wfdA;
			WIN32_FIND_DATAW wfdW;
		};
	#else
		DIR * hFind;
		#if defined(__DSL_USE_READDIR_R__)
			dirent * last_res;
		#endif
	#endif

public:
	Directory();
	Directory(const char * dir); ///< Constructor with directory path (ANSI/UTF-8). This constructor calls Open() with the dir you provide.
	Directory(const wchar_t * dir); ///< Constructor with directory path (Unicode). This constructor calls Open() with the dir you provide.
	~Directory();

	bool Open(const char * dir); ///< Open directory (ANSI/UTF-8)
	bool Open(const wchar_t * dir); ///< Open directory (Unicode)

	/**
	 * Read a directory entry (ANSI/UTF-8)
	 * @param is_dir [optional] Will be set to true if entry is a directory.
	 * @param size [optional] Size of the returned file. On Linux setting this to NULL will save a call to stat().
	 * @return true true on success, false on error or end of the directory.
	 */
	bool Read(char * buf, unsigned long bufSize, bool * is_dir=NULL, int64 * size=NULL);
	/**
	 * Read a directory entry (Unicode)
	 * @param is_dir [optional] Will be set to true if entry is a directory.
	 * @param size [optional] Size of the returned file. On Linux setting this to NULL will save a call to stat().
	 * @return true true on success, false on error or end of the directory.
	 */
	bool Read(wchar_t * buf, unsigned long bufSize, bool * is_dir = NULL, int64 * size = NULL);

	void Close(); ///< Close the directory. Will be called automatically during deconstruction.
};

/**@}*/

#endif // __DSL_DIRECTORY_H__
