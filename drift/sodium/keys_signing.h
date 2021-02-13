//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_SODIUM_KEYS_SIGNING__
#define __DSL_SODIUM_KEYS_SIGNING__

/** \addtogroup sodium
 * @{
 */

#define SIG_PUBKEY_SIZE_BYTES (crypto_sign_PUBLICKEYBYTES)
#define SIG_PRIVKEY_SIZE_BYTES (crypto_sign_SECRETKEYBYTES)
#define SIG_SIZE_BYTES crypto_sign_BYTES

class DSL_SODIUM_API_CLASS DS_SigPubKey {
public:
	uint8_t key[SIG_PUBKEY_SIZE_BYTES];

	DS_SigPubKey();

	string GetString();
	const char * c_str();
	void SetNull();
	bool IsValid();

	bool SetFromHexString(string str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);

	bool operator == (const DS_SigPubKey &b) const {
		return (sodium_memcmp(key, b.key, SIG_PUBKEY_SIZE_BYTES) == 0) ? true : false;
	}
	bool operator != (const DS_SigPubKey &b) const {
		return (sodium_memcmp(key, b.key, SIG_PUBKEY_SIZE_BYTES) != 0) ? true : false;
	}
	bool operator < (const DS_SigPubKey &b) const {
		if (memcmp(key, b.key, SIG_PUBKEY_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

class DSL_SODIUM_API_CLASS DS_SigPrivKey {
private:
	void updatePubKey();
	bool fLocked;
	void checkLocking(bool fForce = false);

public:
	uint8_t key[SIG_PRIVKEY_SIZE_BYTES];
	DS_SigPubKey pubkey;

	DS_SigPrivKey();
	~DS_SigPrivKey();

	string GetString();
	const char * c_str();
	void SetNull();
	bool IsValid();

	bool Generate();

	bool SetFromHexString(string str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);
	bool SetBothFromBinaryData(const uint8_t * sdata, size_t slen, const uint8_t * pdata, size_t plen);

	bool operator == (const DS_SigPrivKey &b) const {
		return (sodium_memcmp(key, b.key, SIG_PRIVKEY_SIZE_BYTES) == 0) ? true : false;
	}
	bool operator != (const DS_SigPrivKey &b) const {
		return (sodium_memcmp(key, b.key, SIG_PRIVKEY_SIZE_BYTES) != 0) ? true : false;
	}
	bool operator < (const DS_SigPrivKey &b) const {
		if (memcmp(key, b.key, SIG_PRIVKEY_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

class DSL_SODIUM_API_CLASS DS_Signature {
private:

public:
	uint8_t hash[SIG_SIZE_BYTES];
	DS_SigPubKey pubkey;

	DS_Signature();

	bool CheckSignature(const uint8_t * msg, size_t msglen);
	bool SignData(DS_SigPrivKey& key, const uint8_t * msg, size_t msglen);

	string GetString();
	const char * c_str();
	void SetNull();
	bool IsNull();
	bool IsValid();

	/* Sets pubkey and signature hash */
	bool SetFromHexString(string str);
	/* Sets pubkey and signature hash */
	bool SetFromBinaryData(const uint8_t * p_hash, size_t len);
	/* Sets just the signature hash */
	bool SetSigFromBinaryData(const uint8_t * p_hash, size_t len);

	/*
	void FromSerialized(const PacketSignature& psig);
	PacketSignature GetSerialized();
	*/

	bool operator == (const DS_Signature &b) const {
		return (sodium_memcmp(hash, b.hash, SIG_SIZE_BYTES) == 0) ? true : false;
	}
	bool operator != (const DS_Signature &b) const {
		return (sodium_memcmp(hash, b.hash, SIG_SIZE_BYTES) != 0) ? true : false;
	}
	bool operator < (const DS_Signature &b) const {
		if (memcmp(hash, b.hash, SIG_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

/**@}*/

#endif // __DSL_SODIUM_KEYS_SIGNING__
