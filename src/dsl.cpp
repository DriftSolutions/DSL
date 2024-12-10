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
#include <drift/GenLib.h>
#include <drift/mutex.h>
#include <drift/threading.h>
#include <drift/os_version.h>
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
#else
	GetTickCount64(); //seeds the start time
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

bool DSL_CC dsl_fill_random_buffer(uint8 * buf, size_t len, bool secure_only) {
	int tries = 0;

#if !defined(WIN32)
	size_t left = len;
	uint8 *p = buf;
#if (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 25))
	while (left > 0 && tries++ < 10) {
		int grn = getrandom(p, left, GRND_NONBLOCK);
		if (grn > 0) {
			left -= grn;
			p += grn;
		} else {
			tries++;
		}
	}
	tries = 0;
#elif defined(SYS_getrandom)
	while (left > 0 && tries++ < 10) {
		int grn = syscall(SYS_getrandom, p, left, GRND_NONBLOCK);
		if (grn > 0) {
			left -= grn;
			p += grn;
		} else {
			tries++;
		}
	}
	tries = 0;
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
	if (left == 0) {
		return true;
	}
#else
	while (tries++ < 10) {
		if (RtlGenRandom(buf, (ULONG)len)) {
			return true;
		}
	}
#endif

#if defined(WIN32) || defined(GCC_RDRAND)
	// try RDRAND if supported
	if (dsl_rdrand(buf, len)) {
		return true;
	}
#endif

	if (!secure_only) {
		// crappy fallback
		for (unsigned int i = 0; i < len; i++) {
			buf[i] = rand() % 256;
		}
		return true;
	}

	return false;
}

#if defined(WIN32) || defined(GCC_RDRAND)
// On Linux this uses malloc somewhere so have to move it down here so it's not throwing #error
#include <immintrin.h>

COMPILE_TIME_ASSERT(sizeof(unsigned int) == 4);

bool DSL_CC dsl_rdrand(uint8 * buf, size_t len) {
	if (!InstructionSet::RDRAND()) {
		return false;
	}
	int tries = 0;
	size_t left = len;
	uint8 *p = buf;

	while (left > 0 && tries < 10) {
#if defined(WIN64) || defined(__x86_64__)
		if (left >= 8) {
			if (_rdrand64_step((uint64_t *)p)) {
				p += sizeof(uint64_t);
				left -= sizeof(uint64_t);
			} else {
				tries++;
			}
		} else
#endif
			if (left >= 4) {
				if (_rdrand32_step((unsigned int *)p)) {
					p += sizeof(unsigned int);
					left -= sizeof(unsigned int);
				} else {
					tries++;
				}
			} else {
				uint16_t tmp;
				if (_rdrand16_step(&tmp)) {
					size_t toGen = (left < sizeof(tmp)) ? left : sizeof(tmp);
					memcpy(p, &tmp, toGen);
					p += toGen;
					left -= toGen;
				} else {
					tries++;
				}
			}
	}

	return (left == 0) ? true : false;
}

#endif // defined(WIN32) || defined(GCC_RDRAND)

char * DSL_CC dsl_mprintf(const char * fmt, ...) {
	va_list va;
	va_start(va, fmt);
	char * ret = dsl_vmprintf(fmt, va);
	va_end(va);
	return ret;
}
string mprintf(const char * fmt, ...) {
	va_list va;
	va_start(va, fmt);
	char * tmp = dsl_vmprintf(fmt, va);
	va_end(va);
	string ret = tmp;
	dsl_free(tmp);
	return ret;
}

char * DSL_CC dsl_vmprintf(const char * fmt, va_list va) {
#if defined(WIN32)
	int len = vscprintf(fmt, va) + 1;
#else
	va_list va_tmp;
	va_copy(va_tmp, va);
	int len = vsnprintf(NULL, 0, fmt, va_tmp) + 1;
	va_end(va_tmp);
#endif
	if (len < 1) {
		return dsl_strdup("vsnprintf error");
	}
	char * ret = (char *)dsl_malloc(len);
	if (vsnprintf(ret, len, fmt, va) < 0) {
		return dsl_strdup("vsnprintf error");
	}
	return ret;
}

wchar_t * DSL_CC dsl_wmprintf(const wchar_t * fmt, ...) {
	va_list va;
	va_start(va, fmt);
#if defined(WIN32)
	int len = vscwprintf(fmt, va);
	wchar_t * ret = (wchar_t *)dsl_malloc((len+1)*sizeof(wchar_t));
	wvsprintfW(ret, fmt, va);
#else
	wchar_t * ret = NULL;
#endif
	va_end(va);
	return ret;
}

#undef malloc
#undef realloc
#undef strdup
#undef wcsdup
#undef free

void * DSL_CC dsl_malloc(size_t lSize) { return malloc(lSize); }
void * DSL_CC dsl_zmalloc(size_t lSize) { return calloc(1, lSize); }
void * DSL_CC dsl_realloc(void * ptr, size_t lSize) { return realloc(ptr, lSize); }
char * DSL_CC dsl_strdup(const char * ptr) { return strdup(ptr); }
wchar_t * DSL_CC dsl_wcsdup(const wchar_t * ptr) { return wcsdup(ptr); }
void DSL_CC dsl_free(void * ptr) { free(ptr); }
void DSL_CC dsl_freep(void ** ptr) {
	if (*ptr != NULL) {
		free(*ptr);
		*ptr = NULL;
	}
}