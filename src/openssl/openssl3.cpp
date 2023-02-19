//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef ENABLE_OPENSSL
#include <drift/dslcore.h>
#if OPENSSL_VERSION_NUMBER >= 0x30000000
#include <drift/hash.h>
#include <drift/hmac.h>

#if defined(WIN32)
extern "C" {
#include <openssl/applink.c> // needed for OpenSSL
}
#endif

HASH_CTX * dsl_openssl_hash_init(const char * name) {
	const EVP_MD * md = EVP_get_digestbyname(name);
	if (md == NULL) {
		return NULL;
	}
	EVP_MD_CTX * ctx = EVP_MD_CTX_create();
	if (EVP_DigestInit_ex(ctx, md, NULL) == 0) {
		EVP_MD_CTX_destroy(ctx);
		return NULL;
	}
	HASH_CTX * ret = dsl_new(HASH_CTX);
	memset(ret, 0, sizeof(HASH_CTX));
	ret->pptr1 = (void *)ctx;
	ret->hashSize = EVP_MD_size(md);
	ret->blockSize = EVP_MD_block_size(md);
	return ret;
}

void dsl_openssl_hash_update(HASH_CTX *ctx, const uint8 *data, size_t len) {
	EVP_MD_CTX * pctx = (EVP_MD_CTX *)ctx->pptr1;
	EVP_DigestUpdate(pctx, data, len);
}

bool dsl_openssl_hash_finish(HASH_CTX *ctx, uint8 * out, size_t outlen) {
	EVP_MD_CTX * pctx = (EVP_MD_CTX *)ctx->pptr1;

	unsigned int slen = outlen;
	bool ret = (EVP_DigestFinal_ex(pctx, out, &slen) != 0);
	EVP_MD_CTX_destroy(pctx);
	dsl_free(ctx);
	return ret;
}

const HASH_PROVIDER openssl_hash_provider = {
	"openssl",
	dsl_openssl_hash_init,
	dsl_openssl_hash_update,
	dsl_openssl_hash_finish
};

HASH_HMAC_CTX * dsl_openssl_hmac_init(const char * name, const uint8 *key, size_t length) {
	EVP_MAC * mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
	if (mac == NULL) {
		return NULL;
	}
	EVP_MAC_CTX * ctx = EVP_MAC_CTX_new(mac);
	if (ctx == NULL) {
		EVP_MAC_free(mac);
		return NULL;
	}
	EVP_MAC_free(mac);

	OSSL_PARAM params[2];
	params[0] = OSSL_PARAM_construct_utf8_string("digest", (char *)name, 0);
	params[1] = OSSL_PARAM_construct_end();

	if (EVP_MAC_init(ctx, key, length, params) == 0) {
		EVP_MAC_CTX_free(ctx);
		return NULL;
	}

	HASH_HMAC_CTX * ret = dsl_new(HASH_HMAC_CTX);
	memset(ret, 0, sizeof(HASH_HMAC_CTX));
	ret->pptr1 = (void *)ctx;
	ret->hashSize = EVP_MAC_CTX_get_mac_size(ctx);
	return ret;
}

void dsl_openssl_hmac_update(HASH_HMAC_CTX *ctx, const uint8 *data, size_t len) {
	EVP_MAC_CTX * pctx = (EVP_MAC_CTX *)ctx->pptr1;
	EVP_MAC_update(pctx, data, len);
}

bool dsl_openssl_hmac_finish(HASH_HMAC_CTX *ctx, uint8 * out, size_t outlen) {
	EVP_MAC_CTX * pctx = (EVP_MAC_CTX *)ctx->pptr1;
	size_t slen = outlen;
	bool ret = (EVP_MAC_final(pctx, out, &slen, outlen) != 0);
	EVP_MAC_CTX_free(pctx);
	dsl_free(ctx);
	return ret;
}

const HMAC_PROVIDER openssl_hmac_provider = {
	"openssl",
	dsl_openssl_hmac_init,
	dsl_openssl_hmac_update,
	dsl_openssl_hmac_finish
};

bool openssl_has_init = false;
#if OPENSSL_VERSION_NUMBER < 0x10100000L

#if defined(OPENSSL_THREADS)
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
void dsl_openssl_idcb(CRYPTO_THREADID * tid) {
	CRYPTO_THREADID_set_numeric(tid, (unsigned long)GetCurrentThreadId());
}
#else
unsigned long dsl_openssl_idcb(void) {
	return (unsigned long)GetCurrentThreadId();
}
#endif

bool openssl_did_locking = false;
DSL_Mutex ** sslMutex;
void dsl_openssl_lockcb(int mode, int type, const char *file, int line) {
	if (mode & CRYPTO_LOCK) {
		sslMutex[type]->Lock();
	} else {
		sslMutex[type]->Release();
	}
}
void setup_openssl_locking() {
	int num = CRYPTO_num_locks();
	sslMutex = (DSL_Mutex **)dsl_malloc(sizeof(DSL_Mutex *)*num);
	for (int i = 0; i < num; i++) {
		sslMutex[i] = new DSL_Mutex;
	}
	CRYPTO_set_locking_callback(dsl_openssl_lockcb);
	openssl_did_locking = true;
}
void shutdown_openssl_locking() {
	int num = CRYPTO_num_locks();
	CRYPTO_set_locking_callback(NULL);
	for (int i = 0; i < num; i++) {
		delete sslMutex[i];
	}
	dsl_free(sslMutex);
	openssl_did_locking = false;
}
#endif // OPENSSL_VERSION_NUMBER < 0x10100000L
#endif // OPENSSL_THREADS

bool dsl_openssl_init() {
	OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);
	uint8 buf[1024];
	while (!RAND_status()) {
		if (dsl_fill_random_buffer(buf, sizeof(buf))) {
			RAND_add(buf, sizeof(buf), sizeof(buf) - 1);
		}
	}
	openssl_has_init = true;

	dsl_add_hash_provider(&openssl_hash_provider);
	dsl_add_hmac_provider(&openssl_hmac_provider);

	printf("DSL: OpenSSL init...\n");
	return true;
}

void dsl_openssl_cleanup() {
	if (openssl_has_init) {
		dsl_remove_hash_provider(&openssl_hash_provider);
		dsl_remove_hmac_provider(&openssl_hmac_provider);
		openssl_has_init = false;
	}
}

bool DSL_CC dsl_openssl_fill_random_buffer(uint8 * buf, size_t len) {
	int tries = 0;

	if (openssl_has_init) {
		tries = 0;
		while (tries++ < 10) {
			if (RAND_bytes(buf, len) >= 0) {
				return true;
			}
		}
	}

	return false;
}

const DSL_LIBRARY_FUNCTIONS dsl_openssl_funcs = {
	false,
	DSL_OPTION_OPENSSL,
	dsl_openssl_init,
	dsl_openssl_cleanup,
	NULL,
};
DSL_Library_Registerer dsl_openssl_autoreg(dsl_openssl_funcs);

#endif // OPENSSL_VERSION_NUMBER >= 0x30000000
#endif // ENABLE_OPENSSL
