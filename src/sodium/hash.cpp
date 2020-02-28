//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifdef ENABLE_SODIUM
#include <drift/dslcore.h>
#include <drift/GenLib.h>
#include <drift/sodium.h>

const uint8_t null_hash_bytes[HASH_SIZE_BYTES] = { 0 };
DS_Hash null_hash;

DS_Hash::DS_Hash() {
	SetNull();
}

vector<uint8_t> DS_Hash::GetVector() {
	vector<uint8_t> ret(&hash[0], &hash[sizeof(hash)]);
	return ret;
}

void DS_Hash::SetNull() {
	//this is used in a static initializer so can't use bin2hex
	memset(hash, 0, HASH_SIZE_BYTES);
}
bool DS_Hash::IsNull() {
	return (sodium_is_zero(&hash[1], sizeof(hash) - 1) == 1);
}
bool DS_Hash::IsValid() {
	return !(sodium_is_zero(&hash[1], sizeof(hash) - 1) == 1);
}

bool DS_Hash::SetFromHexString(string str) {
	if (str.length() == HASH_SIZE_BYTES * 2 && strspn(str.c_str(), "abcdefABCDEF0123456789") == str.length()) {
		hex2bin(str.c_str(), hash, HASH_SIZE_BYTES);
		return IsValid();
	}
	return false;
}
bool DS_Hash::SetFromBinaryData(uint8_t * p_hash, size_t len) {
	if (len == HASH_SIZE_BYTES) {
		memcpy(hash, p_hash, HASH_SIZE_BYTES);
		return IsValid();
	}
	return false;
}

/* DS_Hasher */

DS_Hasher::DS_Hasher() {
	SetNull();
}
DS_Hasher::DS_Hasher(const uint8_t * data, size_t len, uint8_t nVersion) {
	HashData(data, len, nVersion);
}
DS_Hasher::DS_Hasher(vector<uint8_t>& data, uint8_t nVersion) {
	HashData(data, nVersion);
}

DS_Hash DS_Hasher::HashData(const uint8_t * data, size_t len, uint8_t nVersion) {
	crypto_generichash(&hash[1], HASH_SIZE_BYTES - 1, data, len, NULL, 0);
	hash[0] = nVersion;
	return *this;
}
DS_Hash DS_Hasher::HashData(vector<uint8_t>& data, uint8_t nVersion) {
	return HashData(data.data(), data.size(), nVersion);
}

#endif
