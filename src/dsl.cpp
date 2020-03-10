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
#include <drift/GenLib.h>
#include <drift/Mutex.h>
#include <drift/Threading.h>
#include <stdarg.h>
#if defined(WIN32)
#include <tchar.h>
#endif

DSL_Mutex * dslMutex()
{
  static DSL_Mutex actualMutex;
  return &actualMutex;
}

int dsl_init_count = 0;
bool dsl_init_ret = false;
uint32 dsl_runtime_options = 0;

vector<DSL_LIBRARY_FUNCTIONS> dsl_lib_funcs;

void DSL_CC dsl_register_lib(DSL_LIBRARY_FUNCTIONS funcs) {
	AutoMutexPtr(dslMutex());
	if (dsl_init_count > 0 && dsl_init_ret) {
		//if this is somehow called late, init the library now
		funcs.has_init = true;
		dsl_runtime_options |= funcs.nOption;
	}
	dsl_lib_funcs.push_back(funcs);
}
DSL_Library_Registerer::~DSL_Library_Registerer() {}
void DSL_Library_Registerer::EnsureLinked() {}

#if defined(WIN32)
typedef BOOLEAN (APIENTRY *RtlGenRandomType)(PVOID RandomBuffer, ULONG RandomBufferLength);
RtlGenRandomType RGR = NULL;
#endif

bool DSL_CC dsl_init() {
	AutoMutexPtr(dslMutex());
	dsl_init_count++;
#if defined(DEBUG)
	//printf("dsl_init: %d\n", dsl_init_count);
#endif
	if (dsl_init_count > 1) {
		return dsl_init_ret;
	}
	dsl_init_ret = false;

#if defined(WIN32)
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2,2),&wsd) != 0) {
		printf("DSL: Error initializing Winsock!\n");
		return false;
	}

	if (RGR == NULL) {
		HMODULE hRGR = LoadLibrary("advapi32.dll");
		if (hRGR) {
			RGR = (RtlGenRandomType)GetProcAddress(hRGR, "SystemFunction036");
			if (RGR == NULL) {
				FreeLibrary(hRGR);
			}
		}
	}
#endif

	for (auto x = dsl_lib_funcs.begin(); x != dsl_lib_funcs.end(); x++) {
		if (x->init == NULL || x->init()) {
			x->has_init = true;
			dsl_runtime_options |= x->nOption;
		}
	}

	dsl_init_ret = true;
	return true;
}

void dsl_cleanup() {
	AutoMutexPtr(dslMutex());
	if (dsl_init_count > 0) {
		dsl_init_count--;
	}
#if defined(DEBUG)
	//printf("dsl_cleanup: %d\n", dsl_init_count);
#endif
	if (dsl_init_count > 0) {
		return;
	}

	for (auto x = dsl_lib_funcs.begin(); x != dsl_lib_funcs.end(); x++) {
		if (x->has_init && x->cleanup != NULL) {
			x->cleanup();
		}
		x->has_init = false;
		dsl_runtime_options &= ~x->nOption;
	}

#if defined(WIN32)
	WSACleanup();
#endif
}

void DSL_CC dsl_get_version(DSL_VERSION * ver) {
	//DSL_OPTION_SSL
	ver->major = 1;
	ver->minor = 1;
	ver->compile_options = 0;
#if defined(ENABLE_ZLIB)
	ver->compile_options |= DSL_OPTION_ZLIB;
#endif
#if defined(MEMLEAK)
	ver->compile_options |= DSL_OPTION_MEMLEAK;
#endif
	ver->compile_options |= dsl_runtime_options;
}

const char * DSL_CC dsl_get_version_string() {
	AutoMutexPtr(dslMutex());
	static char version[64]={0};

	DSL_VERSION ver;
	dsl_get_version(&ver);
	sprintf(version, "%d.%02d", ver.major, ver.minor);
#if defined(ENABLE_ZLIB)
	sstrcat(version, "-zlib");
#endif
#if defined(MEMLEAK)
	sstrcat(version, "-memleak");
#endif
	if (dsl_runtime_options & DSL_OPTION_OPENSSL) {
		sstrcat(version, "-openssl");
	}
	if (dsl_runtime_options & DSL_OPTION_GNUTLS) {
		sstrcat(version, "-gnutls");
	}
	if (dsl_runtime_options & DSL_OPTION_CURL) {
		sstrcat(version, "-curl");
	}
	if (dsl_runtime_options & DSL_OPTION_MYSQL) {
		sstrcat(version, "-mysql");
	}
	if (dsl_runtime_options & DSL_OPTION_PHYSFS) {
		sstrcat(version, "-physfs");
	}
	if (dsl_runtime_options & DSL_OPTION_SQLITE) {
		sstrcat(version, "-sqlite");
	}
	if (dsl_runtime_options & DSL_OPTION_SODIUM) {
		sstrcat(version, "-sodium");
	}
	return version;
}

bool DSL_CC fill_random_buffer(uint8 * buf, size_t len) {
	int tries = 0;

#if !defined(WIN32)
	size_t left = len;
	uint8 *p = buf;
#if defined(SYS_getrandom)
	while (left > 0 && tries++ < 10) {
		int grn = syscall(SYS_getrandom, p, left, GRND_NONBLOCK);
		if (grn > 0) {
			left -= grn;
			p += grn;
		}
	}
#endif
	while (left > 0 && tries++ < 10) {
		FILE * fp = fopen("/dev/urandom", "rb");
		if (fp == NULL) {
			fp = fopen("/dev/random", "rb");
		}
		if (fp != NULL) {
			while (left > 0) {
				size_t n = fread(p, 1, left, fp);
				if (n > 0) {
					p += n;
					left -= n;
				} else {
					break;
				}
			}
			fclose(fp);
		}
	}
	return (left == 0) ? true:false;
#else
	if (RGR != NULL) {
		while (tries++ < 10) {
			if (RGR(buf, len)) {
				return true;
			}
		}
	}
#endif

	// crappy fallback
	for (unsigned int i=0; i < len; i++) {
		buf[i] = rand()%256;
	}

	return false;
}

#undef malloc
#undef realloc
#undef strdup
#undef wcsdup
#undef free

void * DSL_CC dsl_malloc(size_t lSize) { return malloc(lSize); }
void * DSL_CC dsl_realloc(void * ptr, size_t lSize) { return realloc(ptr, lSize); }
char * DSL_CC dsl_strdup(const char * ptr) { return strdup(ptr); }
wchar_t * DSL_CC dsl_wcsdup(const wchar_t * ptr) { return wcsdup(ptr); }
void DSL_CC dsl_free(void * ptr) { free(ptr); }
char * DSL_CC dsl_mprintf(const char * fmt, ...) {
	va_list va;
	va_start(va, fmt);
	char * ret = dsl_vmprintf(fmt, va);
	va_end(va);
	return ret;
}

char * DSL_CC dsl_vmprintf(const char * fmt, va_list va) {
#if defined(WIN32)
	int len = vscprintf(fmt, va);
	//va_end(va);
	//va_start(fmt, va);
	char * ret = (char *)dsl_malloc(len+1);
	vsprintf(ret, fmt, va);
#else
	char * tmp = NULL, *ret = NULL;
	if (vasprintf(&tmp, fmt, va) != -1 && tmp) {
		ret = dsl_strdup(tmp);
		free(tmp);
	} else {
		ret = dsl_strdup("vasprintf error");
	}
#endif
	return ret;
}

wchar_t * DSL_CC dsl_wmprintf(const wchar_t * fmt, ...) {
	va_list va;
	va_start(va, fmt);
#if defined(WIN32)
	int len = vscwprintf(fmt, va);
	//va_end(va);
	//va_start(fmt, va);
	wchar_t * ret = (wchar_t *)dsl_malloc((len+1)*sizeof(wchar_t));
	wvsprintfW(ret, fmt, va);
#else
	wchar_t * ret = NULL;
#endif
	va_end(va);
	return ret;
}
