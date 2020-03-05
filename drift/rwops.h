//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef __RWOPS_H__
#define __RWOPS_H__

struct DSL_FILE {
	bool (*seek)(DSL_FILE * fp, int64 pos, int mode);//SEEK_SET/SEEK_CUR/SEEK_END
	int64 (*read)(void * buf, int64 size, DSL_FILE * fp);
	int64 (*write)(void * buf, int64 size, DSL_FILE * fp);
	int64 (*tell)(DSL_FILE * fp);
	bool (*flush)(DSL_FILE * fp);
	bool (*eof)(DSL_FILE * fp);
	void (*close)(DSL_FILE * fp);

	// for use by rwops provider, do not touch in user-land
	union {
		FILE * fp;
		void * handle;
	};
	void * p_extra;
};

DSL_API DSL_FILE * DSL_CC RW_OpenFile(const char * fn, const char * mode);
DSL_API DSL_FILE * DSL_CC RW_ConvertFile(FILE * fp, bool autoclose);
DSL_API DSL_FILE * DSL_CC RW_OpenMemory(int64 size);
DSL_API DSL_FILE * DSL_CC RW_ConvertMemory(char * buf, int64 size);

#ifdef ENABLE_PHYSFS
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
	#define DSL_PHYSFS_API
	#define DSL_PHYSFS_API_CLASS
#endif

DSL_PHYSFS_API DSL_FILE * DSL_CC RW_OpenPhysFS(const char * fn, char * mode, bool autoclose);
DSL_PHYSFS_API DSL_FILE * DSL_CC RW_ConvertPhysFS(PHYSFS_file * fp, bool autoclose);
#endif

/* Used internally - not for public use */
struct TP_RWOPT {
	bool autoclose;
};

#endif // __RWOPS_H__
