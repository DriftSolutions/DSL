//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef _DSL_HASH_H_
#define _DSL_HASH_H_

#include <drift/rwops.h>

/**
 * \defgroup hash Hash Functions
 */

/** \addtogroup hash
 * @{
 */

struct HASH_NATIVE;
struct HASH_PROVIDER;
struct HASH_CTX {
	size_t hashSize;
	size_t blockSize;

#ifndef DOXYGEN_SKIP
	const HASH_PROVIDER * provider;
	void *pptr1;
	const HASH_NATIVE * impl;
#endif
};

/**
 * Initialize a hashing CTX with hashing algorithm 'name'<br>
 * Without optional modules we support: sha3-256, sha3-512, sha256, sha512, sha1, md5, keccak256 (the pre-SHA3 version as used by Ethereum), keccak512<br>
 * With ENABLE_OPENSSL: Adds the full range of OpenSSL supported hash algorithms<br>
 * With ENABLE_GNUTLS: Adds RIPEMD-160, MD2, SHA224, SHA384<br>
 * With ENABLE_SODIUM: Adds blake2b<br>
 * See hashdata(), hashfile(), hashfile_fp(), and hashfile_rw() for one-shot convenience functions. The hashfile* ones automatically hash the file in 32K chunks so it won't try to load the entire file into memory or anything.
 * @param name The name of the hashing algorithm.
 */
DSL_API HASH_CTX * DSL_CC hash_init(const char * name);
DSL_API void DSL_CC hash_update(HASH_CTX *ctx, const uint8 *input, size_t length); ///< Call with the data you want to hash, can be called multiple times to hash a large file in chunks for example.
DSL_API bool DSL_CC hash_finish(HASH_CTX *ctx, uint8 * out, size_t outlen); ///< Finalize hash and store in out. outlen should be >= hashSize in the HASH_CTX struct. After this no further calls to hash_update() can be made and ctx is destroyed.

DSL_API bool DSL_CC hashdata(const char * name, const uint8 *data, size_t datalen, char * out, size_t outlen, bool raw_output = false); ///< Wrapper around hash_init()/hash_update()/hash_finish(). If raw_output == true out will contain binary data, otherwise will contain a hex string.
DSL_API bool DSL_CC hashfile(const char * name, const char * fn, char * out, size_t outlen, bool raw_output = false); ///< Wrapper around hash_init()/hash_update()/hash_finish(). If raw_output == true out will contain binary data, otherwise will contain a hex string.
DSL_API bool DSL_CC hashfile_fp(const char * name, FILE * fp, char * out, size_t outlen, bool raw_output = false); ///< Wrapper around hash_init()/hash_update()/hash_finish(). If raw_output == true out will contain binary data, otherwise will contain a hex string.
DSL_API bool DSL_CC hashfile_rw(const char * name, DSL_FILE * fp, char * out, size_t outlen, bool raw_output = false); ///< Wrapper around hash_init()/hash_update()/hash_finish(). If raw_output == true out will contain binary data, otherwise will contain a hex string.

/**@}*/

#ifndef DOXYGEN_SKIP
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
extern const HASH_PROVIDER dsl_native_hashers;
#endif // DOXYGEN_SKIP

#endif // _DSL_HASH_H_
