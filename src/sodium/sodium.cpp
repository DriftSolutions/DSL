//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef ENABLE_SODIUM
#include <drift/dslcore.h>
#include <drift/sodium.h>

HASH_CTX * dsl_sodium_hash_init(const char * name) {
	bool is256 = (stricmp(name, "blake2b") == 0 || stricmp(name, "blake2b-256") == 0 || stricmp(name, "blake2b256") == 0);
	bool is512 = (stricmp(name, "blake2b-512") == 0 || stricmp(name, "blake2b512") == 0);
	if (!is256 && !is512) { return NULL; }
	crypto_generichash_state * ctx = (crypto_generichash_state *)sodium_malloc(crypto_generichash_statebytes());
	if (ctx == NULL) { return NULL; }
	crypto_generichash_init(ctx, NULL, 0, is512 ? 64 : 32);
	HASH_CTX * ret = dsl_znew(HASH_CTX);
	ret->pptr1 = (void *)ctx;
	ret->hashSize = is512 ? 64 : 32;
	ret->blockSize = 1;
	return ret;
}

void dsl_sodium_hash_update(HASH_CTX *ctx, const uint8 *data, size_t len) {
	crypto_generichash_state * pctx = (crypto_generichash_state *)ctx->pptr1;
	crypto_generichash_update(pctx, data, len);
}

bool dsl_sodium_hash_finish(HASH_CTX *ctx, uint8 * out, size_t outlen) {
	crypto_generichash_state * pctx = (crypto_generichash_state *)ctx->pptr1;

	bool ret = false;
	if (outlen >= ctx->hashSize) {
		ret = (crypto_generichash_final(pctx, out, ctx->hashSize) == 0);
	}
	sodium_free(pctx);
	dsl_free(ctx);
	return ret;
}

const HASH_PROVIDER sodium_hash_provider = {
	"libsodium",
	dsl_sodium_hash_init,
	dsl_sodium_hash_update,
	dsl_sodium_hash_finish
};

bool fSodiumInit = false;

bool dsl_sodium_init() {
	if (sodium_init() < 0) {
		return false;
	}
	dsl_add_hash_provider(&sodium_hash_provider);
	fSodiumInit = true;
	return true;
}

DSL_LIBRARY_FUNCTIONS dsl_sodium_funcs = {
	false,
	DSL_OPTION_SODIUM,
	dsl_sodium_init,
	NULL,
	NULL,
};
DSL_Library_Registerer dsl_sodium_autoreg(dsl_sodium_funcs);

#endif
