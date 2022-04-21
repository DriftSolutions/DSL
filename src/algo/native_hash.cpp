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
#include <drift/hash.h>
#include <drift/algo/sha1.h>
#include <drift/algo/sha2.h>
#include <drift/algo/sha3.h>
#include <drift/algo/md5.h>

/* SHA-1 */

bool native_sha1_init(HASH_CTX * ctx) {
	sha1_ctxt * sctx = (sha1_ctxt *)dsl_malloc(sizeof(sha1_ctxt));
	memset(sctx, 0, sizeof(sha1_ctxt));
	sha1_init(sctx);
	ctx->pptr1 = sctx;
	return true;
}

void native_sha1_update(HASH_CTX * ctx, const uint8 *input, size_t length) {
	sha1_loop((sha1_ctxt *)ctx->pptr1, input, length);
}
bool native_sha1_finish(HASH_CTX * ctx, uint8 * out) {
	sha1_result((sha1_ctxt *)ctx->pptr1, out);
	dsl_free(ctx->pptr1);
	return true;
}

HASH_NATIVE hash_sha1 = {
	20,
	64,

	native_sha1_init,
	native_sha1_update,
	native_sha1_finish
};

/* SHA-2 */

bool native_sha256_init(HASH_CTX * ctx) {
	sha256_ctx * sctx = (sha256_ctx *)dsl_malloc(sizeof(sha256_ctx));
	memset(sctx, 0, sizeof(sha256_ctx));
	sha256_init(sctx);
	ctx->pptr1 = sctx;
	return true;
}

void native_sha256_update(HASH_CTX * ctx, const uint8 *input, size_t length) {
	sha256_update((sha256_ctx *)ctx->pptr1, input, length);
}
bool native_sha256_finish(HASH_CTX * ctx, uint8 * out) {
	sha256_final((sha256_ctx *)ctx->pptr1, out);
	dsl_free(ctx->pptr1);
	return true;
}

HASH_NATIVE hash_sha256 = {
	32,
	64,

	native_sha256_init,
	native_sha256_update,
	native_sha256_finish
};

bool native_sha512_init(HASH_CTX * ctx) {
	sha512_ctx * sctx = (sha512_ctx *)dsl_malloc(sizeof(sha512_ctx));
	memset(sctx, 0, sizeof(sha512_ctx));
	sha512_init(sctx);
	ctx->pptr1 = sctx;
	return true;
}

void native_sha512_update(HASH_CTX * ctx, const uint8 *input, size_t length) {
	sha512_update((sha512_ctx *)ctx->pptr1, input, length);
}
bool native_sha512_finish(HASH_CTX * ctx, uint8 * out) {
	sha512_final((sha512_ctx *)ctx->pptr1, out);
	dsl_free(ctx->pptr1);
	return true;
}

HASH_NATIVE hash_sha512 = {
	64,
	128,

	native_sha512_init,
	native_sha512_update,
	native_sha512_finish
};

/* SHA-3 */

bool native_keccak256_init(HASH_CTX * ctx) {
	sha3_context * sha3ctx = (sha3_context *)dsl_malloc(sizeof(sha3_context));
	memset(sha3ctx, 0, sizeof(sha3_context));
	sha3_Init256(sha3ctx);
	sha3_SetFlags(sha3ctx, SHA3_FLAGS_KECCAK);
	ctx->pptr1 = sha3ctx;
	return true;
}

void native_keccak256_update(HASH_CTX * ctx, const uint8 *input, size_t length) {
	sha3_Update((sha3_context *)ctx->pptr1, input, length);
}
bool native_keccak256_finish(HASH_CTX * ctx, uint8 * out) {
	const uint8_t * hash = (const uint8_t *)sha3_Finalize((sha3_context *)ctx->pptr1);
	memcpy(out, hash, ctx->hashSize);
	dsl_free(ctx->pptr1);
	return true;
}

HASH_NATIVE hash_keccak256 = {
	32,
	136,

	native_keccak256_init,
	native_keccak256_update,
	native_keccak256_finish
};

bool native_keccak512_init(HASH_CTX * ctx) {
	sha3_context * sha3ctx = (sha3_context *)dsl_malloc(sizeof(sha3_context));
	memset(sha3ctx, 0, sizeof(sha3_context));
	sha3_Init512(sha3ctx);
	sha3_SetFlags(sha3ctx, SHA3_FLAGS_KECCAK);
	ctx->pptr1 = sha3ctx;
	return true;
}

void native_keccak512_update(HASH_CTX * ctx, const uint8 *input, size_t length) {
	sha3_Update((sha3_context *)ctx->pptr1, input, length);
}
bool native_keccak512_finish(HASH_CTX * ctx, uint8 * out) {
	const uint8_t * hash = (const uint8_t *)sha3_Finalize((sha3_context *)ctx->pptr1);
	memcpy(out, hash, ctx->hashSize);
	dsl_free(ctx->pptr1);
	return true;
}

HASH_NATIVE hash_keccak512 = {
	64,
	72,

	native_keccak512_init,
	native_keccak512_update,
	native_keccak512_finish
};

bool native_sha3_256_init(HASH_CTX * ctx) {
	sha3_context * sha3ctx = (sha3_context *)dsl_malloc(sizeof(sha3_context));
	memset(sha3ctx, 0, sizeof(sha3_context));
	sha3_Init256(sha3ctx);
	ctx->pptr1 = sha3ctx;
	return true;
}

void native_sha3_256_update(HASH_CTX * ctx, const uint8 *input, size_t length) {
	sha3_Update((sha3_context *)ctx->pptr1, input, length);
}
bool native_sha3_256_finish(HASH_CTX * ctx, uint8 * out) {
	const uint8_t * hash = (const uint8_t *)sha3_Finalize((sha3_context *)ctx->pptr1);
	memcpy(out, hash, ctx->hashSize);
	dsl_free(ctx->pptr1);
	return true;
}

HASH_NATIVE hash_sha3_256 = {
	32,
	136,

	native_sha3_256_init,
	native_sha3_256_update,
	native_sha3_256_finish
};

bool native_sha3_512_init(HASH_CTX * ctx) {
	sha3_context * sha3ctx = (sha3_context *)dsl_malloc(sizeof(sha3_context));
	memset(sha3ctx, 0, sizeof(sha3_context));
	sha3_Init512(sha3ctx);
	ctx->pptr1 = sha3ctx;
	return true;
}

void native_sha3_512_update(HASH_CTX * ctx, const uint8 *input, size_t length) {
	sha3_Update((sha3_context *)ctx->pptr1, input, length);
}
bool native_sha3_512_finish(HASH_CTX * ctx, uint8 * out) {
	const uint8_t * hash = (const uint8_t *)sha3_Finalize((sha3_context *)ctx->pptr1);
	memcpy(out, hash, ctx->hashSize);
	dsl_free(ctx->pptr1);
	return true;
}

HASH_NATIVE hash_sha3_512 = {
	64,
	72,

	native_sha3_512_init,
	native_sha3_512_update,
	native_sha3_512_finish
};

/* MD5 */

bool native_md5_init(HASH_CTX * ctx) {
	md5_context * md5ctx = (md5_context *)dsl_malloc(sizeof(md5_context));
	memset(md5ctx, 0, sizeof(md5_context));
	md5_init(md5ctx);
	ctx->pptr1 = md5ctx;
	return true;
}

void native_md5_update(HASH_CTX * ctx, const uint8 *input, size_t length) {
	md5_update((md5_context *)ctx->pptr1, input, length);
}
bool native_md5_finish(HASH_CTX * ctx, uint8 * out) {
	md5_finish((md5_context *)ctx->pptr1, out);
	dsl_free(ctx->pptr1);
	return true;
}

HASH_NATIVE hash_md5 = {
	MD5_HASH_SIZE,
	64,

	native_md5_init,
	native_md5_update,
	native_md5_finish
};

/* Hashing interface */

class HASH_MAP {
public:
	string name;
	const HASH_NATIVE * algo;
};

vector<HASH_MAP> algos {
	{ "md5", &hash_md5 },
	{ "sha1", &hash_sha1 },
	{ "sha-1", &hash_sha1 },
	{ "sha256", &hash_sha256 },
	{ "sha-256", &hash_sha256 },
	{ "sha512", &hash_sha512 },
	{ "sha-512", &hash_sha512 },
	{ "keccak256", &hash_keccak256 },
	{ "keccak512", &hash_keccak512 },
	{ "sha3-256", &hash_sha3_256 },
	{ "sha3-512", &hash_sha3_512 },
};

void DSL_CC dsl_add_native_hash(const char * name, const HASH_NATIVE * p) {
	HASH_MAP m;
	m.name = name;
	m.algo = p;
	algos.push_back(m);
}

HASH_CTX * dsl_native_hash_init(const char * name) {
	HASH_CTX * ret = dsl_new(HASH_CTX);
	memset(ret, 0, sizeof(HASH_CTX));
	for (int i = 0; i < algos.size(); i++) {
		if (!stricmp(name, algos[i].name.c_str())) {
			ret->hashSize = algos[i].algo->hashSize;
			ret->blockSize = algos[i].algo->blockSize;
			ret->impl = algos[i].algo;
			if (algos[i].algo->init(ret)) {
				return ret;
			}
			break;
		}
	}
	dsl_free(ret);
	return NULL;
}

void dsl_native_hash_update(HASH_CTX *ctx, const uint8 *data, size_t len) {
	ctx->impl->update(ctx, data, len);
}

bool dsl_native_hash_finish(HASH_CTX *ctx, uint8 * out, size_t outlen) {
	bool ret = ctx->impl->finish(ctx, out);
	dsl_free(ctx);
	return ret;
}

const HASH_PROVIDER dsl_native_hashers = {
	"native",
	dsl_native_hash_init,
	dsl_native_hash_update,
	dsl_native_hash_finish
};
