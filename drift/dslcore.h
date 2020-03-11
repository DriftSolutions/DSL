//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSLCORE_H__
#define __DSLCORE_H__

#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif
#if !defined(__cplusplus) && !defined(NO_CPLUSPLUS)
#define NO_CPLUSPLUS
#endif
#if defined(ENABLE_SSL) && !defined(ENABLE_OPENSSL)
//compatibility with legacy code
#define ENABLE_OPENSSL
#endif

#ifdef __GNUC__
#define COMPILE_TIME_ASSERT(expr)		static char __attribute__((unused)) MAKE_UNIQUE_NAME[expr];
#else
#define COMPILE_TIME_ASSERT(expr)		static char MAKE_UNIQUE_NAME[expr];
#endif
#define MAKE_UNIQUE_NAME					MAKE_NAME(__LINE__)
#define MAKE_NAME(line)					MAKE_NAME2(line)
#define MAKE_NAME2(line)					constraint_ ## line

#if defined(XBOX)
	#error "The original Xbox is no longer a supported platform"
#elif defined(WIN32) || defined(_WIN32)
	#include <drift/platforms/win32.h>
#elif defined(LINUX) || defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
	#include <drift/platforms/linux.h>
#elif defined(FREEBSD) || defined(__FreeBSD__) || defined(__FreeBSD_cc_version) || defined(__OpenBSD__)
	#include <drift/platforms/freebsd.h>
#elif defined(_APPLE_) || defined(__APPLE_CC__)
	#include <drift/platforms/macosx.h>
#elif defined(SOLARIS) || (defined(__sun__) && defined(__svr4__))
	#include <drift/platforms/solaris.h>
#else
	#warning "ERROR: Unable to detect platform!"
	#error "ERROR: Unable to detect platform!"
#endif

#if !defined(NO_CPLUSPLUS)
#include <sstream>
#include <string>
#include <vector>
#include <map>
using namespace std;
#endif

/**
 * \defgroup dslcore DSL Core Functions & Data Types
 */

/** \addtogroup dslcore
 * @{
 */

#include <drift/types.h>

#if defined(ENABLE_ZLIB)
#include <zlib.h>
#endif
#if defined(ENABLE_CURL)
#include <curl/curl.h>
#endif
#if defined(ENABLE_PHYSFS)
#include <physfs.h>
#endif
#if defined(ENABLE_OPENSSL)
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#endif
#if defined(ENABLE_GNUTLS)
#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>
#endif
#if defined(ENABLE_SODIUM)
#include <sodium.h>
#endif

#define DSL_OPTION_SSL			0x00000001
#define DSL_OPTION_ZLIB			0x00000002
#define DSL_OPTION_MEMLEAK		0x00000004
#define DSL_OPTION_MYSQL		0x00000008
#define DSL_OPTION_PHYSFS		0x00000010
#define DSL_OPTION_CURL			0x00000020
#define DSL_OPTION_SQLITE		0x00000040
#define DSL_OPTION_SODIUM		0x00000080
#define DSL_OPTION_OPENSSL		0x00000100
#define DSL_OPTION_GNUTLS			0x00000200

typedef struct {
	int major, minor;
	uint32 compile_options; ///< See the DSL_OPTION_* defines. Will be a bitmask of 0 or more of them.
} DSL_VERSION;
DSL_API void DSL_CC dsl_get_version(DSL_VERSION * ver); ///< Fills in a DSL_VERSION struct you provide.
DSL_API const char * DSL_CC dsl_get_version_string(); ///< Returns the DSL version string.

/**
 * Call when your app starts to initialize DSL. You can call dsl_init() more than once without issue since it is ref-counted.
 * @sa dsl_cleanup
 */
DSL_API bool DSL_CC dsl_init();
/**
 * Call when your app is exiting or you are done with all DSL functions. Initialization is ref-counted so you need to call it the same number of times you called dsl_init().<br>
 * Windows: We don't have dsl_cleanup as DSL_CC(__stdcall) so it can be used with atexit().
 * @sa dsl_init
 */
DSL_API void dsl_cleanup();
/**
 * Fills a buffer with random data.<br />
 * Windows: Uses RtlGenRandom if available.<br />
 * Linux: Uses the SYS_getrandom syscall if available, or /dev/urandom, or /dev/random (falling back in that order).<br />
 * Both: If all the above fails, falls back to rand() :(
 */
DSL_API bool DSL_CC dsl_fill_random_buffer(uint8 * buf, size_t len);

DSL_API void * DSL_CC dsl_malloc(size_t lSize);
DSL_API void * DSL_CC dsl_realloc(void * ptr, size_t lSize);
DSL_API char * DSL_CC dsl_strdup(const char * ptr);
DSL_API wchar_t * DSL_CC dsl_wcsdup(const wchar_t * ptr);
/**
 * Returns a dynamically allocated string using printf formatting, free with dsl_free.
 * @sa dsl_free
 */
DSL_API char * DSL_CC dsl_mprintf(const char * fmt, ...);
/**
 * Returns a dynamically allocated string using printf formatting, free with dsl_free.
 * @sa dsl_free
 */
DSL_API char * DSL_CC dsl_vmprintf(const char * fmt, va_list va);
/**
 * Returns a dynamically allocated string using printf formatting, free with dsl_free.
 * @sa dsl_free
 */
DSL_API wchar_t * DSL_CC dsl_wmprintf(const wchar_t * fmt, ...);
DSL_API void DSL_CC dsl_free(void * ptr);
/**
 * Call dsl_free on a pointer if it's not NULL
 */
#define dsl_freenn(ptr) if (ptr) { dsl_free(ptr); }
#define dsl_new(x) (x *)dsl_malloc(sizeof(x));

/**@}*/

#ifndef DOXYGEN_SKIP

typedef struct {
	bool has_init; /* has been initialized successfully */
	uint32 nOption;
	bool (*init)();
	void (*cleanup)();
	void * pReserved; /* reserved for future use */
} DSL_LIBRARY_FUNCTIONS;
DSL_API void DSL_CC dsl_register_lib(DSL_LIBRARY_FUNCTIONS funcs);
#if !defined(NO_CPLUSPLUS)
class DSL_API_CLASS DSL_Library_Registerer {
public:
	DSL_Library_Registerer(const DSL_LIBRARY_FUNCTIONS funcs) {
		dsl_register_lib(funcs);
	}

	~DSL_Library_Registerer();
	void EnsureLinked();
};
#endif


#if !defined(DSL_NO_COMPAT) && !defined(DSL_EXPORTS)
#include <drift/compat.h>
#endif

#ifdef DSL_EXPORTS
#define malloc #error
#define realloc #error
#define strdup #error
#define wcsdup #error
#define free #error
#define freenn #error
#endif

#endif // DOXYGEN_SKIP

#endif // __DSLCORE_H__
