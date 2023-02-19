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
#if OPENSSL_VERSION_NUMBER < 0x30000000
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
	const EVP_MD * md = EVP_get_digestbyname(name);
	if (md == NULL) {
		return NULL;
	}

#if OPENSSL_VERSION_NUMBER > 0x10100000
	HMAC_CTX * ctx = HMAC_CTX_new();
	if (ctx == NULL) {
		return NULL;
	}
#else
	HMAC_CTX * ctx = dsl_new(HMAC_CTX);
	if (ctx == NULL) {
		return NULL;
	}
	HMAC_CTX_init(ctx);
#endif
	if (HMAC_Init_ex(ctx, key, length, md, NULL) == 0) {
#if OPENSSL_VERSION_NUMBER > 0x10100000
		HMAC_CTX_free(ctx);
#else
		HMAC_CTX_cleanup(ctx);
		dsl_free(ctx);
#endif
		return NULL;
	}

	HASH_HMAC_CTX * ret = dsl_new(HASH_HMAC_CTX);
	memset(ret, 0, sizeof(HASH_HMAC_CTX));
	ret->pptr1 = (void *)ctx;
	ret->hashSize = EVP_MD_size(md);
	return ret;
}

void dsl_openssl_hmac_update(HASH_HMAC_CTX *ctx, const uint8 *data, size_t len) {
	HMAC_CTX * pctx = (HMAC_CTX *)ctx->pptr1;
	HMAC_Update(pctx, data, len);
}

bool dsl_openssl_hmac_finish(HASH_HMAC_CTX *ctx, uint8 * out, size_t outlen) {
	HMAC_CTX * pctx = (HMAC_CTX *)ctx->pptr1;
	unsigned int slen = outlen;
	bool ret = (HMAC_Final(pctx, out, &slen) != 0);
#if OPENSSL_VERSION_NUMBER > 0x10100000
	HMAC_CTX_free(pctx);
#else
	HMAC_CTX_cleanup(pctx);
	dsl_free(pctx);
#endif
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
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);
#else
	CRYPTO_malloc_init();
#if defined(OPENSSL_THREADS)
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
	if (CRYPTO_THREADID_get_callback() == NULL) {
		CRYPTO_THREADID_set_callback(dsl_openssl_idcb);
	}
#else
	if (CRYPTO_get_id_callback() == NULL) {
		CRYPTO_set_id_callback(dsl_openssl_idcb);
	}
#endif
	if (CRYPTO_get_locking_callback() == NULL) {
		setup_openssl_locking();
	}
#endif
	ERR_load_BIO_strings();
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_digests();
#endif

#ifdef WIN32
	// Seed OpenSSL PRNG with current contents of the screen
	RAND_screen();
#endif
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

#if OPENSSL_VERSION_NUMBER >= 0x10100000L

#else
		RAND_cleanup();
		EVP_cleanup();
		ERR_free_strings();
#if defined(OPENSSL_THREADS)
		if (openssl_did_locking) {
			shutdown_openssl_locking();
		}
#endif // OPENSSL_THREADS
#endif // OPENSSL_VERSION_NUMBER >= 0x10100000L

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

#endif // OPENSSL_VERSION_NUMBER < 0x30000000
#endif // ENABLE_OPENSSL
