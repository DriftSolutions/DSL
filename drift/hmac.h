//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef _DSL_HMAC_H_
#define _DSL_HMAC_H_

#include <drift/rwops.h>

struct HMAC_PROVIDER;
struct HASH_HMAC_CTX {
	size_t hashSize;

	const HMAC_PROVIDER * provider;
	void *pptr1;
};

DSL_API HASH_HMAC_CTX * DSL_CC hmac_init(const char * name, const uint8 *key, size_t length);
DSL_API void DSL_CC hmac_update(HASH_HMAC_CTX *ctx, const uint8 *input, size_t length);
DSL_API bool DSL_CC hmac_finish(HASH_HMAC_CTX *ctx, uint8 * out, size_t outlen);

DSL_API bool DSL_CC hmacdata(const char * name, const uint8 *key, size_t keylen, const uint8 *data, size_t datalen, char * out, size_t outlen);
DSL_API bool DSL_CC hmacfile(const char * name, const uint8 *key, size_t keylen, const char * fn, char * out, size_t outlen);
DSL_API bool DSL_CC hmacfile_fp(const char * name, const uint8 *key, size_t keylen, FILE * fp, char * out, size_t outlen);
DSL_API bool DSL_CC hmacfile_rw(const char * name, const uint8 *key, size_t keylen, DSL_FILE * fp, char * out, size_t outlen);

struct HMAC_PROVIDER {
	const char * const name;
	HASH_HMAC_CTX * (*hmac_init)(const char * name, const uint8 *key, size_t length);
	void(*hmac_update)(HASH_HMAC_CTX *ctx, const uint8 *input, size_t length);
	bool(*hmac_finish)(HASH_HMAC_CTX *ctx, uint8 * out, size_t outlen);
};

DSL_API void DSL_CC dsl_add_hmac_provider(const HMAC_PROVIDER * p);
DSL_API void DSL_CC dsl_remove_hmac_provider(const HMAC_PROVIDER * p);
DSL_API void DSL_CC dsl_get_hmac_providers(vector<const HMAC_PROVIDER *>& p);

#endif // _DSL_HMAC_H_
