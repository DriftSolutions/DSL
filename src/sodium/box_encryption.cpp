//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2024 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef ENABLE_SODIUM
#include <drift/dslcore.h>
#include <drift/GenLib.h>
#include <drift/sodium.h>

void DS_BoxNonce::SetNull() {
	memset(data, 0, sizeof(data));
}

bool DS_BoxNonce::IsValid() const {
	if (sodium_is_zero(data, sizeof(data)) == 1) {
		return false;
	}
	return true;
}

void DS_BoxNonce::Generate() {
	SetNull();
	randombytes_buf(data, BOX_NONCE_SIZE_BYTES);
}

bool DS_BoxNonce::SetFromHexString(const string& str) {
	if (str.length() == BOX_NONCE_SIZE_BYTES * 2 && strspn(str.c_str(), "abcdef0123456789") == str.length()) {
		//strncpy(key_str, str.c_str(), sizeof(key_str));
		hex2bin(str.c_str(), data, BOX_NONCE_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

bool DS_BoxNonce::SetFromBinaryData(const uint8_t * pdata, size_t len) {
	if (len == BOX_NONCE_SIZE_BYTES) {
		memcpy(data, pdata, BOX_NONCE_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

DS_BoxPrivKey::DS_BoxPrivKey() {
	SetNull();
	checkLocking(true);
}

DS_BoxPrivKey::~DS_BoxPrivKey() {
	if (fLocked) {
		sodium_munlock(&key, sizeof(key));
	} else {
		sodium_memzero(key, sizeof(key));
	}
}

void DS_BoxPrivKey::SetNull() {
	//this is used in a static initializer so can't use bin2hex
	sodium_memzero(key, sizeof(key));
}

bool DS_BoxPrivKey::IsValid() const {
	if (sodium_is_zero(key, sizeof(key)) == 1) {
		return false;
	}
	return true;
}

bool DS_BoxPrivKey::Encrypt(const DS_BoxNonce& nonce, DSL_BUFFER * buf) {
	int64 origlen = buf->len;
	buffer_resize(buf, buf->len + crypto_secretbox_MACBYTES);
	return (crypto_secretbox_easy(buf->udata, buf->udata, origlen, nonce.data, key) == 0);
}

bool DS_BoxPrivKey::Decrypt(const DS_BoxNonce& nonce, DSL_BUFFER * buf) {
	if (crypto_secretbox_open_easy(buf->udata, buf->udata, buf->len, nonce.data, key) == 0) {
		buffer_resize(buf, buf->len - crypto_secretbox_MACBYTES);
		return true;
	}
	return false;
}

bool DS_BoxPrivKey::Generate() {
	checkLocking();
	sodium_memzero(&key, sizeof(key));
	crypto_secretbox_keygen(key);
	return IsValid();
}

bool DS_BoxPrivKey::SetFromHexString(const string& str) {
	if (str.length() == BOX_PRIVKEY_SIZE_BYTES * 2 && strspn(str.c_str(), "abcdef0123456789") == str.length()) {
		checkLocking();
		hex2bin(str.c_str(), key, BOX_PRIVKEY_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

bool DS_BoxPrivKey::SetFromBinaryData(const uint8_t * pdata, size_t len) {
	if (len == BOX_PRIVKEY_SIZE_BYTES) {
		checkLocking();
		memcpy(key, pdata, BOX_PRIVKEY_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

void DS_BoxPrivKey::checkLocking(bool fForce) {
	if (fSodiumInit && (!fLocked || fForce)) {
		sodium_mlock(&key, sizeof(key));
		fLocked = true;
	} else {
		fLocked = false;
	}
}

#endif
