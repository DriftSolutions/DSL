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
#include <drift/GenLib.h>
#include <drift/sodium.h>

DS_SigPubKey::DS_SigPubKey() {
	SetNull();
}

void DS_SigPubKey::SetNull() {
	//this is used in a static initializer so can't use bin2hex
	memset(key, 0, sizeof(key));
}

bool DS_SigPubKey::IsValid() const {
	if (sodium_is_zero(key, sizeof(key)) == 1) {
		return false;
	}
	return true;
}

string DS_SigPubKey::GetString() const {
	char key_str[(sizeof(key) * 2) + 1];
	sodium_bin2hex(key_str, sizeof(key_str), key, sizeof(key));
	return key_str;
}

bool DS_SigPubKey::SetFromHexString(string str) {
	if (str.length() == SIG_PUBKEY_SIZE_BYTES * 2 && strspn(str.c_str(), "abcdef0123456789") == str.length()) {
		hex2bin(str.c_str(), key, SIG_PUBKEY_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}
bool DS_SigPubKey::SetFromBinaryData(const uint8_t * pdata, size_t len) {
	if (len == SIG_PUBKEY_SIZE_BYTES) {
		memcpy(key, pdata, SIG_PUBKEY_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

/* DS_SigPrivKey */

void DS_SigPrivKey::updatePubKey() {
	uint8_t tmp[SIG_PUBKEY_SIZE_BYTES];
	if (crypto_sign_ed25519_sk_to_pk(tmp, key) == 0) {
		pubkey.SetFromBinaryData(tmp, SIG_PUBKEY_SIZE_BYTES);
	}
	else {
		pubkey.SetNull();
	}
}

void DS_SigPrivKey::checkLocking(bool fForce) {
}

DS_SigPrivKey::DS_SigPrivKey() {
	SetNull();
	checkLocking(true);
}

DS_SigPrivKey::~DS_SigPrivKey() {
	if (fLocked) {
		sodium_munlock(&key, sizeof(key));
	}
}

string DS_SigPrivKey::GetString() const {
	char key_str[(sizeof(key) * 2) + 1];
	sodium_bin2hex(key_str, sizeof(key_str), key, sizeof(key));
	return key_str;
}
const char * DS_SigPrivKey::c_str() { return GetString().c_str(); }

void DS_SigPrivKey::SetNull() {
	//this is used in a static initializer so can't use bin2hex
	memset(key, 0, sizeof(key));
	pubkey.SetNull();
}
bool DS_SigPrivKey::IsValid() const {
	if (sodium_is_zero(key, sizeof(key)) == 1) {
		return false;
	}
	if (!pubkey.IsValid()) {
		return false;
	}
	/*
	if (memcmp(&key[1], privkey_null_bytes, sizeof(key) - 1) == 0) {
	return false;
	}
	*/
	return true;
}

bool DS_SigPrivKey::Generate() {
	uint8_t pk[SIG_PUBKEY_SIZE_BYTES] = { 0 };
	uint8_t sk[SIG_PRIVKEY_SIZE_BYTES] = { 0 };
	if (crypto_sign_keypair(pk, sk) == 0) {
		SetBothFromBinaryData(sk, sizeof(sk), pk, sizeof(pk));
		sodium_memzero(&sk, sizeof(sk));
		return IsValid();
	}
	return false;
}

bool DS_SigPrivKey::SetFromHexString(string str) {
	if (str.length() == SIG_PRIVKEY_SIZE_BYTES * 2 && strspn(str.c_str(), "abcdef0123456789") == str.length()) {
		checkLocking();
		hex2bin(str.c_str(), key, SIG_PRIVKEY_SIZE_BYTES);
		updatePubKey();
		return IsValid();
	}
	SetNull();
	return false;
}
bool DS_SigPrivKey::SetFromBinaryData(const uint8_t * pdata, size_t len) {
	if (len == SIG_PRIVKEY_SIZE_BYTES) {
		checkLocking();
		memcpy(key, pdata, SIG_PRIVKEY_SIZE_BYTES);
		updatePubKey();
		return IsValid();
	}
	SetNull();
	return false;
}
bool DS_SigPrivKey::SetBothFromBinaryData(const uint8_t * sdata, size_t slen, const uint8_t * pdata, size_t plen) {
	if (slen == SIG_PRIVKEY_SIZE_BYTES) {
		checkLocking();
		memcpy(key, sdata, SIG_PRIVKEY_SIZE_BYTES);
		return pubkey.SetFromBinaryData(pdata, plen);
	}
	SetNull();
	return false;
}


/* DS_Signature */

DS_Signature::DS_Signature() {
	SetNull();
}

bool DS_Signature::CheckSignature(const DS_SigPubKey& pubkey, const uint8_t * msg, size_t msglen) {
	if (!pubkey.IsValid()) {
		return false;
	}
	return (crypto_sign_verify_detached(hash, msg, msglen, pubkey.key) == 0);
}

bool DS_Signature::SignData(DS_SigPrivKey& key, const uint8_t * msg, size_t msglen) {
	SetNull();
	if (!key.IsValid()) {
		return false;
	}
	if (crypto_sign_detached(hash, NULL, msg, msglen, key.key) != 0) {
		return false;
	}
	return IsValid();
}

string DS_Signature::GetString() const {
	char key_str[(sizeof(hash) * 2) + 1];
	sodium_bin2hex(key_str, sizeof(key_str), hash, sizeof(hash));
	//bin2hex(key, sizeof(key), key_str, sizeof(key_str));
	return key_str;
}

void DS_Signature::SetNull() {
	memset(hash, 0, SIG_SIZE_BYTES);
}
bool DS_Signature::IsNull() {
	return (sodium_is_zero(hash, sizeof(hash)) == 1);
	//return (memcmp(hash, sig_null_bytes, sizeof(hash)) == 0);
}
bool DS_Signature::IsValid() const {
	return !(sodium_is_zero(hash, sizeof(hash)) == 1);
}

bool DS_Signature::SetFromHexString(string str) {
	if (str.length() == SIG_SIZE_BYTES * 2 && strspn(str.c_str(), "abcdef0123456789") == str.length()) {
		return (hex2bin(str.c_str(), hash, sizeof(SIG_SIZE_BYTES)) && IsValid());
	}
	SetNull();
	return false;
}

bool DS_Signature::SetFromBinaryData(const uint8_t * p_hash, size_t len) {
	if (len == SIG_SIZE_BYTES) {
		memcpy(hash, p_hash, SIG_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

#endif
