//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef _DSL_HASH_H_
#define _DSL_HASH_H_

#include <drift/rwops.h>

struct HASH_NATIVE;
struct HASH_PROVIDER;
struct HASH_CTX {
	size_t hashSize;
	size_t blockSize;

	const HASH_PROVIDER * provider;
	void *pptr1;
	const HASH_NATIVE * impl;
};

DSL_API HASH_CTX * DSL_CC hash_init(const char * name);
DSL_API void DSL_CC hash_update(HASH_CTX *ctx, const uint8 *input, size_t length);
DSL_API bool DSL_CC hash_finish(HASH_CTX *ctx, uint8 * out, size_t outlen);

DSL_API bool DSL_CC hashdata(const char * name, const uint8 *data, size_t datalen, char * out, size_t outlen, bool raw_output = false);
DSL_API bool DSL_CC hashfile(const char * name, const char * fn, char * out, size_t outlen, bool raw_output = false);
DSL_API bool DSL_CC hashfile_fp(const char * name, FILE * fp, char * out, size_t outlen, bool raw_output = false);
DSL_API bool DSL_CC hashfile_rw(const char * name, DSL_FILE * fp, char * out, size_t outlen, bool raw_output = false);

/* The rest are for internal use only */

struct HASH_PROVIDER {
	const char * const name;
	HASH_CTX * (*hash_init)(const char * name);
	void(*hash_update)(HASH_CTX *ctx, const uint8 *input, size_t length);
	bool(*hash_finish)(HASH_CTX *ctx, uint8 * out, size_t outlen);
};

struct HASH_NATIVE {
	int hashSize;
	int blockSize;

	bool(*init)(HASH_CTX * ctx);
	void(*update)(HASH_CTX * ctx, const uint8 *input, size_t length);
	bool(*finish)(HASH_CTX * ctx, uint8 * out);
};

DSL_API void DSL_CC dsl_add_hash_provider(const HASH_PROVIDER * p);
DSL_API void DSL_CC dsl_remove_hash_provider(const HASH_PROVIDER * p);
DSL_API void DSL_CC dsl_get_hash_providers(vector<const HASH_PROVIDER *>& p);

DSL_API void DSL_CC dsl_add_native_hash(const char * name, const HASH_NATIVE * p); /* This should be called directly after dsl_init() and before any other DSL functions. The pointer must remain valid until after dsl_cleanup() is called. */

#endif // _DSL_HASH_H_
