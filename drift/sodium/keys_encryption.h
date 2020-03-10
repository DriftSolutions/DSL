//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DS_KEYS__ENCRYPTION__
#define __DS_KEYS__ENCRYPTION__

#define ENC_PUBKEY_SIZE_BYTES (crypto_box_PUBLICKEYBYTES + 1)
#define ENC_PRIVKEY_SIZE_BYTES (crypto_box_SECRETKEYBYTES + 1)
#define ENC_NONCE_SIZE_BYTES crypto_box_NONCEBYTES
#define ENC_SHARED_KEY_BYTE_SIZE crypto_box_BEFORENMBYTES
#define ENC_NONCE_INT_TYPE uint32_t

#if ENC_PUBKEY_SIZE_BYTES < ENC_NONCE_SIZE_BYTES
#error Uh-oh
#endif
#if ENC_PUBKEY_SIZE_BYTES < randombytes_SEEDBYTES
#error Uh-oh
#endif

// public keys start with Z and private keys start with C
#define ENC_VER_PUBKEY 0x5A
#define ENC_VER_PRIVKEY 0x43

class DSL_SODIUM_API_CLASS DS_EncNonce {
public:
	uint8_t data[ENC_NONCE_SIZE_BYTES];

	DS_EncNonce();
	void SetNull();
	bool IsValid();

	void Generate();
	ENC_NONCE_INT_TYPE GetIncrement();
	void SetIncrement(ENC_NONCE_INT_TYPE x);
	void Increment();

	bool SetFromHexString(string str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);

	bool operator == (const DS_EncNonce &b) const {
		return (sodium_memcmp(data, b.data, ENC_NONCE_SIZE_BYTES) == 0) ? true : false;
	}
	bool operator != (const DS_EncNonce &b) const {
		return (sodium_memcmp(data, b.data, ENC_NONCE_SIZE_BYTES) != 0) ? true : false;
	}
	bool operator < (const DS_EncNonce &b) const {
		if (memcmp(data, b.data, ENC_NONCE_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

class DS_EncPubKey;
class DS_EncPrivKey;

class DSL_SODIUM_API_CLASS DS_EncSharedKey {
private:
	bool fLocked;
	void checkLocking(bool fForce = false);

public:
	uint8_t data[ENC_SHARED_KEY_BYTE_SIZE];

	DS_EncSharedKey();
	~DS_EncSharedKey();

	void SetNull();
	bool IsValid();

	bool Calculate(DS_EncPrivKey& privkey, DS_EncPubKey& pubkey);
	bool Encrypt(DS_EncNonce& nonce, DSL_BUFFER * buf);
	bool Decrypt(DS_EncNonce& nonce, DSL_BUFFER * buf);

	bool operator == (const DS_EncSharedKey &b) const {
		return (sodium_memcmp(data, b.data, ENC_SHARED_KEY_BYTE_SIZE) == 0) ? true : false;
	}
	bool operator != (const DS_EncSharedKey &b) const {
		return (sodium_memcmp(data, b.data, ENC_SHARED_KEY_BYTE_SIZE) != 0) ? true : false;
	}
	bool operator < (const DS_EncSharedKey &b) const {
		if (memcmp(data, b.data, ENC_SHARED_KEY_BYTE_SIZE) < 0) {
			return true;
		}
		return false;
	}
};

class DSL_SODIUM_API_CLASS DS_EncPubKey {
public:
	uint8_t key[ENC_PUBKEY_SIZE_BYTES];

	DS_EncPubKey();

	string GetString();
	void SetNull();
	bool IsValid();

	bool SetFromHexString(string str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);

	bool operator == (const DS_EncPubKey &b) const {
		return (sodium_memcmp(key, b.key, ENC_PUBKEY_SIZE_BYTES) == 0) ? true : false;
	}
	bool operator != (const DS_EncPubKey &b) const {
		return (sodium_memcmp(key, b.key, ENC_PUBKEY_SIZE_BYTES) != 0) ? true : false;
	}
	bool operator < (const DS_EncPubKey &b) const {
		if (memcmp(key, b.key, ENC_PUBKEY_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

class DSL_SODIUM_API_CLASS DS_EncPrivKey {
private:
	void updatePubKey();
	bool fLocked;
	void checkLocking(bool fForce = false);

public:
	uint8_t key[ENC_PRIVKEY_SIZE_BYTES];
	DS_EncPubKey pubkey;

	DS_EncPrivKey();
	~DS_EncPrivKey();

	string GetString();
	void SetNull();
	bool IsValid();

	bool Encrypt(DS_EncPubKey& recpt_key, DS_EncNonce& nonce, DSL_BUFFER * buf);
	bool Decrypt(DS_EncPubKey& sender_key, DS_EncNonce& nonce, DSL_BUFFER * buf);
	bool Generate();

	bool SetFromHexString(string str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);
	bool SetBothFromBinaryData(const uint8_t * sdata, size_t slen, const uint8_t * pdata, size_t plen);

	bool operator == (const DS_EncPrivKey &b) const {
		return (sodium_memcmp(key, b.key, ENC_PRIVKEY_SIZE_BYTES) == 0) ? true : false;
	}
	bool operator != (const DS_EncPrivKey &b) const {
		return (sodium_memcmp(key, b.key, ENC_PRIVKEY_SIZE_BYTES) != 0) ? true : false;
	}
	bool operator < (const DS_EncPrivKey &b) const {
		if (memcmp(key, b.key, ENC_PRIVKEY_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

#endif // __DS_KEYS__ENCRYPTION__
