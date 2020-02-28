//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

// version byte + 32 byte BLAKE2b hash
#define HASH_SIZE_BYTES (crypto_generichash_BYTES + 1)

class DSL_SODIUM_API_CLASS DS_Hash {
public:
	uint8_t hash[HASH_SIZE_BYTES];

	DS_Hash();

	vector<uint8_t> GetVector();

	void SetNull();
	bool IsNull();
	bool IsValid();

	bool SetFromHexString(string str);
	bool SetFromBinaryData(uint8_t * p_hash, size_t len);

	bool operator == (const DS_Hash &b) const {
		return (memcmp(hash, b.hash, HASH_SIZE_BYTES) == 0) ? true : false;
	}
	bool operator != (const DS_Hash &b) const {
		return (memcmp(hash, b.hash, HASH_SIZE_BYTES) != 0) ? true : false;
	}
	bool operator < (const DS_Hash &b) const {
		if (memcmp(hash, b.hash, HASH_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};
extern DSL_SODIUM_API_CLASS DS_Hash null_hash;

class DSL_SODIUM_API_CLASS DS_Hasher : public DS_Hash {
public:

	DS_Hasher();
	DS_Hasher(const uint8_t * data, size_t len, uint8_t nVersion);
	DS_Hasher(vector<uint8_t>& data, uint8_t nVersion);

	DS_Hash HashData(const uint8_t * data, size_t len, uint8_t nVersion);
	DS_Hash HashData(vector<uint8_t>& data, uint8_t nVersion);
};
