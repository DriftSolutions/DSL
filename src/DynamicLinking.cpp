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
#include <drift/DynamicLinking.h>
#include <drift/WhereIs.h>

DL_HANDLE DSL_CC DL_FindAndOpen(const char * fn) {
	WHEREIS_RESULTS * res = WhereIs(fn);
	DL_HANDLE hDL = NULL;
	if (res) {
		for (int i=0; i < res->nCount && hDL == NULL; i++) {
			hDL = DL_Open(res->sResults[i]);
		}
		WhereIs_FreeResults(res);
	}
	return hDL;
}

#ifdef WIN32

DL_HANDLE DSL_CC DL_Open(const char * fn) {
	return LoadLibraryA(fn);
}

void * DSL_CC DL_GetAddress(DL_HANDLE hHandle, const char * name) {
	return GetProcAddress(hHandle,name);
}

void DSL_CC DL_Close(DL_HANDLE hHandle) {
	FreeLibrary(hHandle);
}

static char dl_error_buf[256];
const char * DSL_CC DL_LastError() {
	memset(dl_error_buf, 0, sizeof(dl_error_buf));
	if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_SYSTEM_DEFAULT, dl_error_buf, sizeof(dl_error_buf)-1, NULL) == 0) {
		strcpy(dl_error_buf, "Error getting error message!");
	}
	return dl_error_buf;
}

#else

DL_HANDLE DL_Open(const char * fn) {
#ifdef FREEBSD
	HANDLE hHandle = dlopen(fn,RTLD_LAZY|RTLD_LOCAL);
	if (hHandle == NULL) {
		hHandle = dlopen(fn,RTLD_NOW|RTLD_LOCAL);
	}
#else
	HANDLE hHandle = dlopen(fn,RTLD_LAZY|RTLD_LOCAL|RTLD_DEEPBIND);
	if (hHandle == NULL) {
		hHandle = dlopen(fn,RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND);
	}
#endif
	return hHandle;
}

void * DL_GetAddress(DL_HANDLE hHandle, const char * name) {
	return dlsym(hHandle,name);
}

void DL_Close(DL_HANDLE hHandle) {
	dlclose(hHandle);
}

const char * DL_LastError() {
	return dlerror();
}

#endif
