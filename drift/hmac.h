//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef _DSL_HMAC_H_
#define _DSL_HMAC_H_

#include <drift/rwops.h>

/**
 * \defgroup hmac HMAC Functions
 */

/** \addtogroup hmac
 * @{
 */

struct HMAC_PROVIDER;
struct HASH_HMAC_CTX {
	size_t hashSize;

#ifndef DOXYGEN_SKIP
	const HMAC_PROVIDER * provider;
	void *pptr1;
#endif
};

/**
 * Initialize a HMAC CTX with hashing algorithm 'name'<br>
 * This feature relies on having external libraries enabled:<br>
 * With ENABLE_OPENSSL: Adds the full range of OpenSSL supported hash algorithms<br>
 * With ENABLE_GNUTLS: Adds sha256, sha512, sha1, md5, RIPEMD-160, MD2, SHA224, SHA384<br>
 * See hmacdata(), hmacfile(), hmacfile_fp(), and hmacfile_rw() for one-shot convenience functions. The hmacfile* ones automatically HMAC the file in 32K chunks so it won't try to load the entire file into memory or anything.
 * @param name The name of the hashing algorithm.
 * @param key The secret key to use.
 * @param length The length of the secret key.
 */
DSL_API HASH_HMAC_CTX * DSL_CC hmac_init(const char * name, const uint8 *key, size_t length);
DSL_API void DSL_CC hmac_update(HASH_HMAC_CTX *ctx, const uint8 *input, size_t length); ///< Call with the data you want to hash, can be called multiple times to hash a large file in chunks for example.
DSL_API bool DSL_CC hmac_finish(HASH_HMAC_CTX *ctx, uint8 * out, size_t outlen); ///< Finalize HMAC and store in out. outlen should be >= hashSize in the HASH_CTX struct. After this no further calls to hmac_update() can be made and ctx is destroyed.

DSL_API bool DSL_CC hmacdata(const char * name, const uint8 *key, size_t keylen, const uint8 *data, size_t datalen, char * out, size_t outlen); ///< Wrapper around hmac_init()/hmac_update()/hmac_finish(). If raw_output == true out will contain binary data, otherwise will contain a hex string.
DSL_API bool DSL_CC hmacfile(const char * name, const uint8 *key, size_t keylen, const char * fn, char * out, size_t outlen); ///< Wrapper around hmac_init()/hmac_update()/hmac_finish(). If raw_output == true out will contain binary data, otherwise will contain a hex string.
DSL_API bool DSL_CC hmacfile_fp(const char * name, const uint8 *key, size_t keylen, FILE * fp, char * out, size_t outlen); ///< Wrapper around hmac_init()/hmac_update()/hmac_finish(). If raw_output == true out will contain binary data, otherwise will contain a hex string.
DSL_API bool DSL_CC hmacfile_rw(const char * name, const uint8 *key, size_t keylen, DSL_FILE * fp, char * out, size_t outlen); ///< Wrapper around hmac_init()/hmac_update()/hmac_finish(). If raw_output == true out will contain binary data, otherwise will contain a hex string.

/**@}*/

#ifndef DOXYGEN_SKIP

struct HMAC_PROVIDER {
	const char * const name;
	HASH_HMAC_CTX * (*hmac_init)(const char * name, const uint8 *key, size_t length);
	void(*hmac_update)(HASH_HMAC_CTX *ctx, const uint8 *input, size_t length);
	bool(*hmac_finish)(HASH_HMAC_CTX *ctx, uint8 * out, size_t outlen);
};

DSL_API void DSL_CC dsl_add_hmac_provider(const HMAC_PROVIDER * p);
DSL_API void DSL_CC dsl_remove_hmac_provider(const HMAC_PROVIDER * p);
DSL_API void DSL_CC dsl_get_hmac_providers(vector<const HMAC_PROVIDER *>& p);

#endif // DOXYGEN_SKIP

#endif // _DSL_HMAC_H_
