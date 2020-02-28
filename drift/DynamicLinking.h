//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifdef WIN32
	#define DL_HANDLE HINSTANCE
	#define DL_SOEXT "dll"
#else
	#define DL_HANDLE HANDLE
	#define DL_SOEXT "so"
#endif

DSL_API DL_HANDLE DSL_CC DL_Open(const char * fn);
DSL_API DL_HANDLE DSL_CC DL_FindAndOpen(const char * fn);
DSL_API void * DSL_CC DL_GetAddress(DL_HANDLE hHandle, const char * name);
DSL_API void DSL_CC DL_Close(DL_HANDLE hHandle);
DSL_API const char * DSL_CC DL_LastError();
