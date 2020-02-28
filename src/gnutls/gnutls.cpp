//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifdef ENABLE_GNUTLS
#include <drift/dslcore.h>
#include <drift/hash.h>
#include <drift/hmac.h>

gnutls_digest_algorithm_t dsl_gnutls_hash_get_by_name(const char * name) {
	if (!stricmp(name, "md5")) {
		return GNUTLS_DIG_MD5;
	}
	if (!stricmp(name, "sha1") || !stricmp(name, "sha-1")) {
		return GNUTLS_DIG_SHA1;
	}
	if (!stricmp(name, "sha256") || !stricmp(name, "sha-256")) {
		return GNUTLS_DIG_SHA256;
	}
	if (!stricmp(name, "MD2")) {
		return GNUTLS_DIG_MD2;
	}
	if (!stricmp(name, "RMD160") || !stricmp(name, "RIPEMD-160")) {
		return GNUTLS_DIG_RMD160;
	}
	if (!stricmp(name, "SHA384") || !stricmp(name, "SHA-384")) {
		return GNUTLS_DIG_SHA384;
	}
	if (!stricmp(name, "SHA224") || !stricmp(name, "SHA-224")) {
		return GNUTLS_DIG_SHA224;
	}
	if (!stricmp(name, "SHA512") || !stricmp(name, "SHA-512")) {
		return GNUTLS_DIG_SHA512;
	}
	return GNUTLS_DIG_UNKNOWN;
}

int dsl_gnutls_hash_block_size(gnutls_digest_algorithm_t alg) {
	switch (alg) {
		case GNUTLS_DIG_MD5:
		case GNUTLS_DIG_SHA1:
		case GNUTLS_DIG_SHA256:
		case GNUTLS_DIG_SHA224:
			return 64;
			break;
		case GNUTLS_DIG_SHA384:
		case GNUTLS_DIG_SHA512:
			return 128;
			break;
		case GNUTLS_DIG_MD2:
			return 16;
			break;
		case GNUTLS_DIG_RMD160:
			return 20;
			break;
	}
	return 64;
}

HASH_CTX * dsl_gnutls_hash_init(const char * name) {
	gnutls_digest_algorithm_t alg = dsl_gnutls_hash_get_by_name(name);
	if (alg == GNUTLS_DIG_UNKNOWN) {
		return NULL;
		/*
		printf("DSL: Unsupported hash algorithm '%s', defaulting to SHA256...\n");
		alg = GNUTLS_DIG_SHA256;
		*/
	}

	gnutls_hash_hd_t ctx;
	if (gnutls_hash_init(&ctx, alg) != 0) {
		return NULL;
	}

	HASH_CTX * ret = dsl_new(HASH_CTX);
	memset(ret, 0, sizeof(HASH_CTX));
	ret->pptr1 = ctx;
	ret->hashSize = gnutls_hash_get_len(alg);
	ret->blockSize = dsl_gnutls_hash_block_size(alg);
	return ret;
}

void dsl_gnutls_hash_update(HASH_CTX *ctx, const uint8 *data, size_t len) {
	gnutls_hash_hd_t pctx = (gnutls_hash_hd_t)ctx->pptr1;
	gnutls_hash(pctx, data, len);
}

bool dsl_gnutls_hash_finish(HASH_CTX *ctx, uint8 * out, size_t outlen) {
	gnutls_hash_hd_t pctx = (gnutls_hash_hd_t)ctx->pptr1;
	bool ret = true;
	if (outlen < ctx->hashSize) {
		printf("DSL: Output buffer too small in hash_finish()!\n");
		char * tmp = (char *)dsl_malloc(ctx->hashSize);
		gnutls_hash_deinit(pctx, tmp);
		memcpy(out, tmp, outlen);
		dsl_free(tmp);
		ret = false;
	} else {
		gnutls_hash_deinit(pctx, out);
	}
	dsl_free(ctx);
	return ret;
}

const HASH_PROVIDER gnutls_hash_provider = {
	"gnutls",
	dsl_gnutls_hash_init,
	dsl_gnutls_hash_update,
	dsl_gnutls_hash_finish
};

gnutls_mac_algorithm_t dsl_gnutls_hmac_get_by_name(const char * name) {
	if (!stricmp(name, "md5")) {
		return GNUTLS_MAC_MD5;
	}
	if (!stricmp(name, "sha1") || !stricmp(name, "sha-1")) {
		return GNUTLS_MAC_SHA1;
	}
	if (!stricmp(name, "sha256") || !stricmp(name, "sha-256")) {
		return GNUTLS_MAC_SHA256;
	}
	if (!stricmp(name, "MD2")) {
		return GNUTLS_MAC_MD2;
	}
	if (!stricmp(name, "RMD160") || !stricmp(name, "RIPEMD-160")) {
		return GNUTLS_MAC_RMD160;
	}
	if (!stricmp(name, "SHA384") || !stricmp(name, "SHA-384")) {
		return GNUTLS_MAC_SHA384;
	}
	if (!stricmp(name, "SHA224") || !stricmp(name, "SHA-224")) {
		return GNUTLS_MAC_SHA224;
	}
	if (!stricmp(name, "SHA512") || !stricmp(name, "SHA-512")) {
		return GNUTLS_MAC_SHA512;
	}
	return GNUTLS_MAC_UNKNOWN;
}


HASH_HMAC_CTX * dsl_gnutls_hmac_init(const char * name, const uint8 *key, size_t length) {
	gnutls_mac_algorithm_t alg = dsl_gnutls_hmac_get_by_name(name);
	if (alg == GNUTLS_MAC_UNKNOWN) {
		return NULL;
		/*
		printf("DSL: Unsupported hash algorithm '%s', defaulting to SHA256...\n");
		alg = GNUTLS_MAC_SHA256;
		*/
	}

	gnutls_hmac_hd_t ctx;
	if (gnutls_hmac_init(&ctx, alg, key, length) != 0) {
		return NULL;
	}

	HASH_HMAC_CTX * ret = dsl_new(HASH_HMAC_CTX);
	memset(ret, 0, sizeof(HASH_HMAC_CTX));
	ret->pptr1 = ctx;
	ret->hashSize = gnutls_hmac_get_len(alg);
	return ret;
}

void dsl_gnutls_hmac_update(HASH_HMAC_CTX *ctx, const uint8 *data, size_t len) {
	gnutls_hmac_hd_t pctx = (gnutls_hmac_hd_t)ctx->pptr1;
	gnutls_hmac(pctx, data, len);
}

bool dsl_gnutls_hmac_finish(HASH_HMAC_CTX *ctx, uint8 * out, size_t outlen) {
	gnutls_hmac_hd_t pctx = (gnutls_hmac_hd_t)ctx->pptr1;
	if (outlen < ctx->hashSize) {
		printf("DSL: Output buffer too small in hmac_finish()!\n");
		char * tmp = (char *)malloc(ctx->hashSize);
		gnutls_hmac_deinit(pctx, tmp);
		memcpy(out, tmp, outlen);
		free(tmp);
	} else {
		gnutls_hmac_deinit(pctx, out);
	}
	dsl_free(ctx);
	return true;
}

const HMAC_PROVIDER gnutls_hmac_provider = {
	"gnutls",
	dsl_gnutls_hmac_init,
	dsl_gnutls_hmac_update,
	dsl_gnutls_hmac_finish
};

bool gnutls_has_init = false;

bool dsl_gnutls_init() {
	if (gnutls_global_init() != GNUTLS_E_SUCCESS) {
		printf("DSL: Error initializing GnuTLS, some functions will be disabled...\n");
		return false;
	}
	gnutls_has_init = true;

	dsl_add_hash_provider(&gnutls_hash_provider);
	dsl_add_hmac_provider(&gnutls_hmac_provider);

	printf("DSL: GnuTLS init...\n");
	return true;
}

void dsl_gnutls_cleanup() {
	if (gnutls_has_init) {
		dsl_remove_hash_provider(&gnutls_hash_provider);
		dsl_remove_hmac_provider(&gnutls_hmac_provider);

		gnutls_global_deinit();

		gnutls_has_init = false;
	}
}

bool DSL_CC dsl_gnutls_fill_random_buffer(uint8 * buf, size_t len) {
	/*
	int tries = 0;

	if (gnutls_has_init) {
		tries = 0;
		while (tries++ < 10) {
			if (RAND_bytes(buf, len) >= 0) {
				return true;
			}
		}
	}
	*/

	return false;
}

const DSL_LIBRARY_FUNCTIONS dsl_gnutls_funcs = {
	false,
	DSL_OPTION_GNUTLS,
	dsl_gnutls_init,
	dsl_gnutls_cleanup,
	NULL,
};
DSL_Library_Registerer dsl_gnutls_autoreg(dsl_gnutls_funcs);

#endif // ENABLE_GNUTLS
