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

DS_SigPubKey::DS_SigPubKey() {
	SetNull();
}

void DS_SigPubKey::SetNull() {
	//this is used in a static initializer so can't use bin2hex
	memset(key, 0, sizeof(key));
}

bool DS_SigPubKey::IsValid() {
	if (key[0] != SIG_VER_PUBKEY) {
		return false;
	}
	if (sodium_is_zero(&key[1], sizeof(key) - 1) == 1) {
		return false;
	}
	return true;
}

string DS_SigPubKey::GetString() {
	char key_str[(sizeof(key) * 2) + 1];
	sodium_bin2hex(key_str, sizeof(key_str), key, sizeof(key));
	//bin2hex(key, sizeof(key), key_str, sizeof(key_str));
	return key_str;
}
const char * DS_SigPubKey::c_str() { return GetString().c_str(); }

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
	if (crypto_sign_ed25519_sk_to_pk(&tmp[1], &key[1]) == 0) {
		tmp[0] = SIG_VER_PUBKEY;
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

string DS_SigPrivKey::GetString() {
	char key_str[(sizeof(key) * 2) + 1];
	sodium_bin2hex(key_str, sizeof(key_str), key, sizeof(key));
	//bin2hex(key, sizeof(key), key_str, sizeof(key_str));
	return key_str;
}
const char * DS_SigPrivKey::c_str() { return GetString().c_str(); }

void DS_SigPrivKey::SetNull() {
	//this is used in a static initializer so can't use bin2hex
	memset(key, 0, sizeof(key));
	pubkey.SetNull();
}
bool DS_SigPrivKey::IsValid() {
	if (key[0] != SIG_VER_PRIVKEY) {
		return false;
	}
	if (sodium_is_zero(&key[1], sizeof(key) - 1) == 1) {
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
	if (crypto_sign_keypair(&pk[1], &sk[1]) == 0) {
		sk[0] = SIG_VER_PRIVKEY;
		pk[0] = SIG_VER_PUBKEY;
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

bool DS_Signature::CheckSignature(const uint8_t * msg, size_t msglen) {
	if (!pubkey.IsValid()) {
		return false;
	}
	return (crypto_sign_verify_detached(hash, msg, msglen, &pubkey.key[1]) == 0);
}

bool DS_Signature::SignData(DS_SigPrivKey& key, const uint8_t * msg, size_t msglen) {
	uint8_t hash[SIG_SIZE_BYTES] = { 0 };
	if (!key.IsValid()) {
		return false;
	}
	if (crypto_sign_detached(hash, NULL, msg, msglen, &key.key[1]) != 0) {
		return false;
	}
	pubkey = key.pubkey;
	return SetSigFromBinaryData(hash, sizeof(hash));
}

string DS_Signature::GetString() {
	char key_str[(sizeof(hash) * 2) + 1];
	sodium_bin2hex(key_str, sizeof(key_str), hash, sizeof(hash));
	//bin2hex(key, sizeof(key), key_str, sizeof(key_str));
	return key_str;
}
const char * DS_Signature::c_str() { return GetString().c_str(); }

void DS_Signature::SetNull() {
	//this is used in a static initializer so can't use bin2hex
	memset(hash, 0, SIG_SIZE_BYTES);
	pubkey.SetNull();
}
bool DS_Signature::IsNull() {
	return (sodium_is_zero(hash, sizeof(hash)) == 1);
	//return (memcmp(hash, sig_null_bytes, sizeof(hash)) == 0);
}
bool DS_Signature::IsValid() {
	return !(sodium_is_zero(hash, sizeof(hash)) == 1);
}

bool DS_Signature::SetFromHexString(string str) {
	if (str.length() == (SIG_PUBKEY_SIZE_BYTES + SIG_SIZE_BYTES) * 2 && strspn(str.c_str(), "abcdef0123456789") == str.length()) {
		uint8_t buf[SIG_PUBKEY_SIZE_BYTES + SIG_SIZE_BYTES];
		hex2bin(str.c_str(), buf, sizeof(buf));
		return SetFromBinaryData(buf, sizeof(buf));
	}
	SetNull();
	return false;
}
bool DS_Signature::SetFromBinaryData(const uint8_t * p_hash, size_t len) {
	if (len == SIG_PUBKEY_SIZE_BYTES + SIG_SIZE_BYTES) {
		if (pubkey.SetFromBinaryData(p_hash, SIG_PUBKEY_SIZE_BYTES)) {
			return SetSigFromBinaryData(p_hash + SIG_PUBKEY_SIZE_BYTES, SIG_SIZE_BYTES);
		}
	}
	SetNull();
	return false;
}
bool DS_Signature::SetSigFromBinaryData(const uint8_t * p_hash, size_t len) {
	if (len == SIG_SIZE_BYTES) {
		memcpy(hash, p_hash, SIG_SIZE_BYTES);
		return IsValid();
	}
	SetNull();
	return false;
}

#endif
