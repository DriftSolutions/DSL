//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#include <drift/dslcore.h>
#include <drift/GenLib.h>
#include <drift/Mutex.h>
#include <drift/hash.h>
#include <drift/hmac.h>

extern DSL_Mutex * dslMutex();
vector<const HMAC_PROVIDER *> hmac_providers;
void DSL_CC dsl_add_hmac_provider(const HMAC_PROVIDER * p) {
	AutoMutexPtr(dslMutex());
	hmac_providers.push_back(p);
}
void DSL_CC dsl_remove_hmac_provider(const HMAC_PROVIDER * p) {
	AutoMutexPtr(dslMutex());
	for (auto x = hmac_providers.begin(); x != hmac_providers.end(); x++) {
		if (*x == p) {
			hmac_providers.erase(x);
			break;
		}
	}
}
void DSL_CC dsl_get_hmac_providers(vector<const HMAC_PROVIDER *>& p) {
	AutoMutexPtr(dslMutex());
	p = hmac_providers;
}


HASH_HMAC_CTX * DSL_CC hmac_init(const char * name, const uint8 *key, size_t length) {
	//AutoMutexPtr(dslMutex());
	for (auto x = hmac_providers.begin(); x != hmac_providers.end(); x++) {
		HASH_HMAC_CTX * ret = (*x)->hmac_init(name, key, length);
		if (ret != NULL) {
			ret->provider = *x;
			return ret;
		}
	}
	return NULL;
}

void DSL_CC hmac_update(HASH_HMAC_CTX *ctx, const uint8 *data, size_t len) {
	if (ctx == NULL || data == NULL) {
		return;
	}
	ctx->provider->hmac_update(ctx, data, len);
}

bool DSL_CC hmac_finish(HASH_HMAC_CTX *ctx, uint8 * out, size_t outlen) {
	if (ctx == NULL || out == NULL) {
		return false;
	}
	return ctx->provider->hmac_finish(ctx, out, outlen);
}

DSL_API bool DSL_CC hmacdata(const char * name, const uint8 *key, size_t keylen, const uint8 *data, size_t datalen, char * out, size_t outlen) {
	HASH_HMAC_CTX * ctx = hmac_init(name, key, keylen);
	if (ctx == NULL) {
		return false;
	}
	if (outlen < ((ctx->hashSize*2)+1)) {
		return false;
	}

	hmac_update(ctx,data,datalen);

	size_t size = ctx->hashSize;
	unsigned char * hashtmp = (unsigned char *)dsl_malloc(size);
	bool ret = hmac_finish(ctx, hashtmp, size);
	if (ret) {
		ret = (bin2hex(hashtmp, size, out, outlen) != NULL);
	}
	dsl_free(hashtmp);
	return ret;
}

DSL_API bool DSL_CC hmacfile(const char * name, const uint8 *key, size_t keylen, const char * fn, char * out, size_t outlen) {
	DSL_FILE * fp = RW_OpenFile(fn, "rb");
	if (fp) {
		bool ret = hmacfile_rw(name, key, keylen, fp, out, outlen);
		fp->close(fp);
		return ret;
	}
	return false;
}

DSL_API bool DSL_CC hmacfile_fp(const char * name, const uint8 *key, size_t keylen, FILE * fpp, char * out, size_t outlen) {
	if (fpp == NULL) { return false; }

	DSL_FILE * fp = RW_ConvertFile(fpp, false);
	if (fp) {
		bool ret = hmacfile_rw(name, key, keylen, fp, out, outlen);
		fp->close(fp);
		return ret;
	}
	return false;
}

DSL_API bool DSL_CC hmacfile_rw(const char * name, const uint8 *key, size_t keylen, DSL_FILE * fp, char * out, size_t outlen) {
	HASH_HMAC_CTX * ctx = hmac_init(name, key, keylen);
	if (ctx == NULL) {
		return false;
	}
	if (outlen < ((ctx->hashSize*2)+1)) {
		return false;
	}

	char buf[32768];

	fp->seek(fp, 0, SEEK_END);
	int64 left = fp->tell(fp);
	fp->seek(fp, 0, SEEK_SET);

	bool ret = true;
	while (left) {
		int64 toRead = (left >= sizeof(buf)) ? sizeof(buf):left;
		/*
		if (toRead > INT_MAX) {
			toRead = INT_MAX;
		}
		*/
		if (fp->read(buf, toRead, fp) == toRead) {
			hmac_update(ctx,(uint8 *)&buf,toRead);
			left -= toRead;
		} else {
			ret = false;
			break;
		}
	}

	int size = ctx->hashSize;
	unsigned char * hashtmp = (unsigned char *)dsl_malloc(ctx->hashSize);
	bool ret2 = hmac_finish(ctx, hashtmp, size);
	if (ret2) {
		ret2 = (bin2hex(hashtmp, size, out, outlen) != NULL);
	}
	dsl_free(hashtmp);
	return (ret && ret2);
}
