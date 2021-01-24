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
#include <drift/GenLib.h>
#include <drift/hash.h>
#include <drift/Mutex.h>

vector<const HASH_PROVIDER *> hash_providers { &dsl_native_hashers };

extern DSL_Mutex * dslMutex();
void DSL_CC dsl_add_hash_provider(const HASH_PROVIDER * p) {
	AutoMutexPtr(dslMutex());
	//hash_providers.push_back(p);
	/* 3rd party providers will probably be more optimized than our generic native ones, so put them 1st */
	hash_providers.insert(hash_providers.begin(), p);
}
void DSL_CC dsl_remove_hash_provider(const HASH_PROVIDER * p) {
	AutoMutexPtr(dslMutex());
	for (auto x = hash_providers.begin(); x != hash_providers.end(); x++) {
		if (*x == p) {
			hash_providers.erase(x);
			break;
		}
	}
}
void DSL_CC dsl_get_hash_providers(vector<const HASH_PROVIDER *>& p) {
	AutoMutexPtr(dslMutex());
	p = hash_providers;
}

HASH_CTX * DSL_CC hash_init(const char * name) {
	//AutoMutexPtr(dslMutex());
	for (auto x = hash_providers.begin(); x != hash_providers.end(); x++) {
		HASH_CTX * ret = (*x)->hash_init(name);
		if (ret != NULL) {
			ret->provider = *x;
			return ret;
		}
	}
	return NULL;
}

void DSL_CC hash_update(HASH_CTX *ctx, const uint8 *data, size_t len) {
	if (ctx == NULL || data == NULL) {
		return;
	}
	ctx->provider->hash_update(ctx, data, len);
}
bool DSL_CC hash_finish(HASH_CTX *ctx, uint8 * out, size_t outlen) {
	if (ctx == NULL || out == NULL) {
		return false;
	}
	if (outlen < ctx->hashSize) {
		printf("DSL: Output buffer too small in hash_finish()!\n");
		uint8 * tmp = (uint8 *)dsl_malloc(ctx->hashSize);
		bool ret = ctx->provider->hash_finish(ctx, tmp, ctx->hashSize);
		memcpy(out, tmp, outlen);
		dsl_free(tmp);
		return ret;
	}
	return ctx->provider->hash_finish(ctx, out, outlen);
}

DSL_API bool DSL_CC hashdata(const char * name, const uint8 *data, size_t datalen, char * out, size_t outlen, bool raw_output) {
	HASH_CTX * ctx = hash_init(name);
	if (ctx == NULL) {
		return false;
	}
	if (raw_output && outlen < ctx->hashSize) {
		return false;
	} else if (!raw_output && outlen < ((ctx->hashSize * 2) + 1)) {
		return false;
	}

	hash_update(ctx,data,datalen);

	bool ret;
	if (raw_output) {
		ret = hash_finish(ctx, (uint8 *)out, outlen);
	} else {
		uint8 * hashtmp = (uint8 *)dsl_malloc(ctx->hashSize);
		size_t hsize = ctx->hashSize;
		ret = hash_finish(ctx, hashtmp, ctx->hashSize);
		if (ret) {
			ret = (bin2hex(hashtmp, hsize, out, outlen) != NULL);
		}
		dsl_free(hashtmp);
	}
	return ret;
}

DSL_API bool DSL_CC hashfile(const char * name, const char * fn, char * out, size_t outlen, bool raw_output) {
	DSL_FILE * fp = RW_OpenFile(fn, "rb");
	if (fp) {
		bool ret = hashfile_rw(name, fp, out, outlen, raw_output);
		fp->close(fp);
		return ret;
	}
	return false;
}

DSL_API bool DSL_CC hashfile_fp(const char * name, FILE * fpp, char * out, size_t outlen, bool raw_output) {
	if (fpp == NULL) { return false; }

	DSL_FILE * fp = RW_ConvertFile(fpp, false);
	if (fp) {
		bool ret = hashfile_rw(name, fp, out, outlen, raw_output);
		fp->close(fp);
		return ret;
	}
	return false;
}

DSL_API bool DSL_CC hashfile_rw(const char * name, DSL_FILE * fp, char * out, size_t outlen, bool raw_output) {
	HASH_CTX * ctx = hash_init(name);
	if (ctx == NULL) {
		return false;
	}
	if (raw_output && outlen < ctx->hashSize) {
		return false;
	} else if (!raw_output && outlen < ((ctx->hashSize * 2) + 1)) {
		return false;
	}

	char buf[32768];

	fp->seek(fp, 0, SEEK_END);
	int64 left = fp->tell(fp);
	fp->seek(fp, 0, SEEK_SET);

	bool ret = true;
	while (left) {
		int64 toRead = (left >= sizeof(buf)) ? sizeof(buf):left;
		if (fp->read(buf, toRead, fp) == toRead) {
			hash_update(ctx,(uint8 *)&buf,toRead);
			left -= toRead;
		} else {
			ret = false;
			break;
		}
	}

	bool ret2;
	if (raw_output) {
		ret2 = hash_finish(ctx, (uint8 *)out, outlen);
	} else {
		size_t hsize = ctx->hashSize;
		unsigned char * hashtmp = (unsigned char *)dsl_malloc(ctx->hashSize);
		ret2 = hash_finish(ctx, hashtmp, ctx->hashSize);
		if (ret2) {
			ret2 = (bin2hex(hashtmp, hsize, out, outlen) != NULL);
		}
		dsl_free(hashtmp);
	}
	return (ret && ret2);
}
