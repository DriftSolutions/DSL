//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __RWOPS_H__
#define __RWOPS_H__

/**
 * \defgroup rwops Abstract File/Memory Reading/Writing
 */

/** \addtogroup rwops
 * @{
 */

struct DSL_FILE {
	/**
	 * Seek within the file.
	 * @param fp The file handle
	 * @param pos The position to seek to, in accordance with what you set the mode to.
	 * @param mode SEEK_SET/SEEK_CUR/SEEK_END as in fseek
	 */
	bool (*seek)(DSL_FILE * fp, int64 pos, int mode);
	/**
	 * Read data from the file.
	 * @return The number of bytes read. 0 on EOF, <0 on failure.
	 */
	int64 (*read)(void * buf, int64 size, DSL_FILE * fp);
	/**
	 * Write data to the file.
	 * @return The number of bytes written. 0 on EOF, <0 on failure.
	 */
	int64 (*write)(void * buf, int64 size, DSL_FILE * fp);
	/**
	 * The current position of the pointer within the file.
	 */
	int64 (*tell)(DSL_FILE * fp);
	bool (*flush)(DSL_FILE * fp);
	/**
	 * Are we at the end of the file?
	 */
	bool (*eof)(DSL_FILE * fp);
	/**
	 * Close the handle when you are done.
	 */
	void (*close)(DSL_FILE * fp);

#ifndef DOXYGEN_SKIP
	// for use by rwops provider, do not touch in user-land
	union {
		FILE * fp;
		void * handle;
	};
	void * p_extra;
#endif
};

/**
 * Opens a file handle.
 * @param fn The filename of the file to open.
 * @param mode The mode, passed directly to fopen.
 * @return A handle on success, NULL on failure.
 */
DSL_API DSL_FILE * DSL_CC RW_OpenFile(const char * fn, const char * mode);
/**
 * Opens a file handle from a FILE handle.
 * @param fp The FILE handle to convert.
 * @param autoclose If true we will call fclose() on your FILE handle for you when you close the DSL_FILE handle.
 * @return A handle on success, NULL on failure.
 */
DSL_API DSL_FILE * DSL_CC RW_ConvertFile(FILE * fp, bool autoclose);
/**
 * Creates a handle for a specified amount of memory.
 * @param size The amount of ram to use.
 * @return A handle on success, NULL on failure.
 */
DSL_API DSL_FILE * DSL_CC RW_OpenMemory(int64 size);
/**
 * Creates a handle for a user-supplied region of memory.
 * @return A handle on success, NULL on failure.
 */
DSL_API DSL_FILE * DSL_CC RW_ConvertMemory(uint8 * buf, int64 size);

#if defined(ENABLE_PHYSFS) || defined(DOXYGEN_SKIP)
#include <physfs.h>

#if defined(DSL_DLL) && defined(WIN32)
	#if defined(DSL_PHYSFS_EXPORTS)
		#define DSL_PHYSFS_API extern "C" __declspec(dllexport)
		#define DSL_PHYSFS_API_CLASS __declspec(dllexport)
	#else
		#define DSL_PHYSFS_API extern "C" __declspec(dllimport)
		#define DSL_PHYSFS_API_CLASS __declspec(dllimport)
	#endif
#else
	#define DSL_PHYSFS_API DSL_API_VIS
	#define DSL_PHYSFS_API_CLASS DSL_API_VIS
#endif

/**
 * Opens a file handle for a file in your PhysicsFS filesystem.
 * @param fn The filename of the file to open.
 * @param mode The mode. PHYSFS_openRead if "r" is in the string, PHYSFS_openWrite if "w" is in the string, PHYSFS_openAppend if "a" is in the string.
 * @return A handle on success, NULL on failure.
 */
DSL_PHYSFS_API DSL_FILE * DSL_CC RW_OpenPhysFS(const char * fn, const char * mode);
/**
 * Opens a file handle from a PHYSFS_file handle.
 * @param fp The PHYSFS_file handle to convert.
 * @param autoclose If true we will call PHYSFS_close() on your PHYSFS_file handle for you when you close the DSL_FILE handle.
 * @return A handle on success, NULL on failure.
 */
DSL_PHYSFS_API DSL_FILE * DSL_CC RW_ConvertPhysFS(PHYSFS_file * fp, bool autoclose);
#endif

/**@}*/

#ifndef DOXYGEN_SKIP

/* Used internally - not for public use */
struct TP_RWOPT {
	bool autoclose;
};

#endif // DOXYGEN_SKIP

#endif // __RWOPS_H__
