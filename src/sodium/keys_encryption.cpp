//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef ENABLE_SODIUM
#include <drift/dslcore.h>
#include <drift/GenLib.h>
#include <drift/sodium.h>

/**
DS_EncNonce: Nonce class for encryption and decryption

The way we are using the nonce for the P2P network is to seed it with the 1st ENC_NONCE_SIZE_BYTES bytes of a hash of the 2 peers public keys.
Then we just generate 8 bytes of randomness at the beginning of the nonce and send only those 8 bytes to eachother instead of the whole larger buffer.
**/
DS_EncNonce::DS_EncNonce() {
	SetNull();
}

void DS_EncNonce::SetNull() {
	memset(data, 0, sizeof(data));
}

bool DS_EncNonce::IsValid() {
	if (sodium_is_zero(data, sizeof(data)) == 1) {
		return false;
	}
	return true;
}

void DS_EncNonce::Generate() {
	randombytes_buf(data, ENC_NONCE_SIZE_BYTES);
}

ENC_NONCE_INT_TYPE DS_EncNonce::GetIncrement() {
	ENC_NONCE_INT_TYPE ret = 0;
	memcpy(&ret, data, sizeof(ENC_NONCE_INT_TYPE));
	return ret;
}

void DS_EncNonce::SetIncrement(ENC_NONCE_INT_TYPE x) {
	memcpy(data, &x, sizeof(ENC_NONCE_INT_TYPE));
}

void DS_EncNonce::Increment() {
	randombytes_buf(data, sizeof(ENC_NONCE_INT_TYPE));
}

bool DS_EncNonce::SetFromHexString(string str) {
	if (str.length() == ENC_NONCE_SIZE_BYTES * 2 && strspn(str.c_str(), "abcdef0123456789") == str.length()) {
		//strncpy(key_str, str.c_str(), sizeof(key_str));
		hex2bin(str.c_str(), data, ENC_NONCE_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

bool DS_EncNonce::SetFromBinaryData(const uint8_t * pdata, size_t len) {
	if (len == ENC_NONCE_SIZE_BYTES) {
		memcpy(data, pdata, ENC_NONCE_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

/**
DS_EncSharedKey: Shared Encryption key for Encryption/Decryption

A shared key is generated from your private key and the other peer's public key. This is to save CPU time versus calculating the shared key on each packet using DS_EncPrivKey
*/
DS_EncSharedKey::DS_EncSharedKey() {
	SetNull();
	checkLocking(true);
}

DS_EncSharedKey::~DS_EncSharedKey() {
	if (fLocked) {
		sodium_munlock(data, sizeof(data));
	} else {
		sodium_memzero(data, sizeof(data));
	}
}

void DS_EncSharedKey::SetNull() {
	memset(data, 0, sizeof(data));
}

bool DS_EncSharedKey::IsValid() {
	if (sodium_is_zero(data, sizeof(data)) == 1) {
		return false;
	}
	return true;
}

bool DS_EncSharedKey::Calculate(DS_EncPrivKey& privkey, DS_EncPubKey& pubkey) {
	checkLocking();
	if (crypto_box_beforenm(data, &pubkey.key[1], &privkey.key[1]) == 0) {
		return IsValid();
	}
	return false;
}

bool DS_EncSharedKey::Encrypt(DS_EncNonce& nonce, DSL_BUFFER * buf) {
		DSL_BUFFER tmp;
		buffer_init(&tmp);
		buffer_set(&tmp, buf->data, buf->len);
		buffer_resize(buf, buf->len + crypto_box_MACBYTES);
		if (crypto_box_easy_afternm(buf->udata, tmp.udata, tmp.len, nonce.data, data) == 0) {
			buffer_free(&tmp);
			return true;
		}
		buffer_free(&tmp);
		return false;
}

bool DS_EncSharedKey::Decrypt(DS_EncNonce& nonce, DSL_BUFFER * buf) {
		DSL_BUFFER tmp;
		buffer_init(&tmp);
		buffer_set(&tmp, buf->data, buf->len);
		buffer_resize(buf, buf->len - crypto_box_MACBYTES);
		if (crypto_box_open_easy_afternm(buf->udata, tmp.udata, tmp.len, nonce.data, data) == 0) {
			buffer_free(&tmp);
			return true;
		}
		buffer_free(&tmp);
		return false;
}

void DS_EncSharedKey::checkLocking(bool fForce) {
	if (fSodiumInit && (!fLocked || fForce)) {
		sodium_mlock(&data, sizeof(data));
		fLocked = true;
	} else {
		fLocked = false;
	}
}

void DS_EncPrivKey::checkLocking(bool fForce) {
	if (fSodiumInit && (!fLocked || fForce)) {
		sodium_mlock(&key, sizeof(key));
		fLocked = true;
	} else {
		fLocked = false;
	}
}

/**
DS_EncPubKey
*/

DS_EncPubKey::DS_EncPubKey() {
	SetNull();
}

void DS_EncPubKey::SetNull() {
	//this is used in a static initializer so can't use bin2hex
	sodium_memzero(key, sizeof(key));
}

bool DS_EncPubKey::IsValid() {
	if (key[0] != ENC_VER_PUBKEY) {
		return false;
	}
	if (sodium_is_zero(&key[1], sizeof(key) - 1) == 1) {
		return false;
	}
	return true;
}

bool DS_EncPubKey::SetFromHexString(string str) {
	if (str.length() == ENC_PUBKEY_SIZE_BYTES * 2 && strspn(str.c_str(), "abcdef0123456789") == str.length()) {
		hex2bin(str.c_str(), key, ENC_PUBKEY_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

bool DS_EncPubKey::SetFromBinaryData(const uint8_t * pdata, size_t len) {
	if (len == ENC_PUBKEY_SIZE_BYTES) {
		memcpy(key, pdata, ENC_PUBKEY_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

/**
DS_EncPriKey: Encryption private key and encryption interface
*/

void DS_EncPrivKey::updatePubKey() {
	uint8_t tmp[ENC_PUBKEY_SIZE_BYTES];
	if (crypto_scalarmult_base(&tmp[1], &key[1]) == 0) {
		tmp[0] = ENC_VER_PUBKEY;
		pubkey.SetFromBinaryData(tmp, ENC_PUBKEY_SIZE_BYTES);
	} else {
		pubkey.SetNull();
	}
}

DS_EncPrivKey::DS_EncPrivKey() {
	SetNull();
	checkLocking(true);
}

DS_EncPrivKey::~DS_EncPrivKey() {
	if (fLocked) {
		sodium_munlock(&key, sizeof(key));
	} else {
		sodium_memzero(key, sizeof(key));
	}
}

void DS_EncPrivKey::SetNull() {
	//this is used in a static initializer so can't use bin2hex
	sodium_memzero(key, sizeof(key));
	pubkey.SetNull();
}

bool DS_EncPrivKey::IsValid() {
	if (key[0] != ENC_VER_PRIVKEY) {
		return false;
	}
	if (sodium_is_zero(&key[1], sizeof(key) - 1) == 1) {
		return false;
	}
	if (!pubkey.IsValid()) {
		return false;
	}
	return true;
}

bool DS_EncPrivKey::Encrypt(DS_EncPubKey& recpt_key, DS_EncNonce& nonce, DSL_BUFFER * buf) {
	DSL_BUFFER tmp;
	buffer_init(&tmp);
	buffer_set(&tmp, buf->data, buf->len);
	buffer_resize(buf, buf->len + crypto_box_MACBYTES);
	if (crypto_box_easy(buf->udata, tmp.udata, tmp.len, nonce.data, &recpt_key.key[1], &key[1]) == 0) {
		buffer_free(&tmp);
		return true;
	}
	buffer_free(&tmp);
	return false;
}

bool DS_EncPrivKey::Decrypt(DS_EncPubKey& sender_key, DS_EncNonce& nonce, DSL_BUFFER * buf) {
	DSL_BUFFER tmp;
	buffer_init(&tmp);
	buffer_set(&tmp, buf->data, buf->len);
	buffer_resize(buf, buf->len - crypto_box_MACBYTES);
	if (crypto_box_open_easy(buf->udata, tmp.udata, tmp.len, nonce.data, &sender_key.key[1], &key[1]) == 0) {
		buffer_free(&tmp);
		return true;
	}
	buffer_free(&tmp);
	return false;
}

bool DS_EncPrivKey::Generate() {
	uint8_t pk[ENC_PUBKEY_SIZE_BYTES] = { 0 };
	uint8_t sk[ENC_PRIVKEY_SIZE_BYTES] = { 0 };
	if (crypto_box_keypair(&pk[1], &sk[1]) == 0) {
		sk[0] = ENC_VER_PRIVKEY;
		pk[0] = ENC_VER_PUBKEY;
		SetBothFromBinaryData(sk, sizeof(sk), pk, sizeof(pk));
		sodium_memzero(&sk, sizeof(sk));
		return IsValid();
	}
	return false;
}

bool DS_EncPrivKey::SetFromHexString(string str) {
	if (str.length() == ENC_PRIVKEY_SIZE_BYTES * 2 && strspn(str.c_str(), "abcdef0123456789") == str.length()) {
		checkLocking();
		hex2bin(str.c_str(), key, ENC_PRIVKEY_SIZE_BYTES);
		updatePubKey();
		return IsValid();
	}
	SetNull();
	return false;
}

bool DS_EncPrivKey::SetFromBinaryData(const uint8_t * pdata, size_t len) {
	if (len == ENC_PRIVKEY_SIZE_BYTES) {
		checkLocking();
		memcpy(key, pdata, ENC_PRIVKEY_SIZE_BYTES);
		updatePubKey();
		return IsValid();
	}
	SetNull();
	return false;
}

bool DS_EncPrivKey::SetBothFromBinaryData(const uint8_t * sdata, size_t slen, const uint8_t * pdata, size_t plen) {
	if (slen == ENC_PRIVKEY_SIZE_BYTES) {
		checkLocking();
		memcpy(key, sdata, ENC_PRIVKEY_SIZE_BYTES);
		return pubkey.SetFromBinaryData(pdata, plen);
	}
	SetNull();
	return false;
}

#endif
