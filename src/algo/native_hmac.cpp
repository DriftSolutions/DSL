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
#include <drift/hmac.h>
#include <drift/algo/hmac_sha2.h>

/* SHA-2 */

bool native_hmac_sha256_init(HASH_HMAC_CTX * ctx, const uint8 *key, size_t length) {
	hmac_sha256_ctx * sctx = (hmac_sha256_ctx *)dsl_malloc(sizeof(hmac_sha256_ctx));
	memset(sctx, 0, sizeof(hmac_sha256_ctx));
	hmac_sha256_init(sctx, key, length);
	ctx->pptr1 = sctx;
	return true;
}

void native_hmac_sha256_update(HASH_HMAC_CTX * ctx, const uint8 *input, size_t length) {
	hmac_sha256_update((hmac_sha256_ctx *)ctx->pptr1, input, length);
}
bool native_hmac_sha256_finish(HASH_HMAC_CTX * ctx, uint8 * out) {
	hmac_sha256_final((hmac_sha256_ctx *)ctx->pptr1, out, ctx->hashSize);
	dsl_free(ctx->pptr1);
	return true;
}

HMAC_NATIVE hash_hmac_sha256 = {
	32,

	native_hmac_sha256_init,
	native_hmac_sha256_update,
	native_hmac_sha256_finish
};

bool native_hmac_sha512_init(HASH_HMAC_CTX * ctx, const uint8 *key, size_t length) {
	hmac_sha512_ctx * sctx = (hmac_sha512_ctx *)dsl_malloc(sizeof(hmac_sha512_ctx));
	memset(sctx, 0, sizeof(hmac_sha512_ctx));
	hmac_sha512_init(sctx, key, length);
	ctx->pptr1 = sctx;
	return true;
}

void native_hmac_sha512_update(HASH_HMAC_CTX * ctx, const uint8 *input, size_t length) {
	hmac_sha512_update((hmac_sha512_ctx *)ctx->pptr1, input, length);
}
bool native_hmac_sha512_finish(HASH_HMAC_CTX * ctx, uint8 * out) {
	hmac_sha512_final((hmac_sha512_ctx *)ctx->pptr1, out, ctx->hashSize);
	dsl_free(ctx->pptr1);
	return true;
}

HMAC_NATIVE hash_hmac_sha512 = {
	64,

	native_hmac_sha512_init,
	native_hmac_sha512_update,
	native_hmac_sha512_finish
};

/* Hashing interface */

class HMAC_MAP {
public:
	string name;
	const HMAC_NATIVE * algo;
};

vector<HMAC_MAP> hmac_algos {
	{ "sha256", &hash_hmac_sha256 },
	{ "sha-256", &hash_hmac_sha256 },
	{ "sha512", &hash_hmac_sha512 },
	{ "sha-512", &hash_hmac_sha512 },
};

void DSL_CC dsl_add_native_hmac(const char * name, const HMAC_NATIVE * p) {
	HMAC_MAP m;
	m.name = name;
	m.algo = p;
	hmac_algos.push_back(m);
}

HASH_HMAC_CTX * dsl_native_hmac_init(const char * name, const uint8 *key, size_t length) {
	HASH_HMAC_CTX * ret = dsl_new(HASH_HMAC_CTX);
	memset(ret, 0, sizeof(HASH_HMAC_CTX));
	for (int i = 0; i < hmac_algos.size(); i++) {
		if (!stricmp(name, hmac_algos[i].name.c_str())) {
			ret->hashSize = hmac_algos[i].algo->hashSize;
			ret->impl = hmac_algos[i].algo;
			if (hmac_algos[i].algo->init(ret, key, length)) {
				return ret;
			}
			break;
		}
	}
	dsl_free(ret);
	return NULL;
}

void dsl_native_hmac_update(HASH_HMAC_CTX *ctx, const uint8 *data, size_t len) {
	ctx->impl->update(ctx, data, len);
}

bool dsl_native_hmac_finish(HASH_HMAC_CTX *ctx, uint8 * out, size_t outlen) {
	bool ret = ctx->impl->finish(ctx, out);
	dsl_free(ctx);
	return ret;
}

const HMAC_PROVIDER dsl_native_hmacers = {
	"native",
	dsl_native_hmac_init,
	dsl_native_hmac_update,
	dsl_native_hmac_finish
};
