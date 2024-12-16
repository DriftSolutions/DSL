//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_SODIUM_KEYS_ENCRYPTION__
#define __DSL_SODIUM_KEYS_ENCRYPTION__

/** \addtogroup sodium
 * @{
 */

#define ENC_PUBKEY_SIZE_BYTES (crypto_box_PUBLICKEYBYTES)
#define ENC_PRIVKEY_SIZE_BYTES (crypto_box_SECRETKEYBYTES)
#define ENC_NONCE_SIZE_BYTES crypto_box_NONCEBYTES
#define ENC_SHARED_KEY_BYTE_SIZE crypto_box_BEFORENMBYTES

#if ENC_PUBKEY_SIZE_BYTES < ENC_NONCE_SIZE_BYTES
#error Uh-oh
#endif
#if ENC_PUBKEY_SIZE_BYTES < randombytes_SEEDBYTES
#error Uh-oh
#endif

/**
 * A nonce used for encrypting & decrypting data with DS_EncPrivKey, DS_EncPubKey, and DS_EncSharedKey.
 */
class DSL_SODIUM_API_CLASS DS_EncNonce {
public:
	uint8_t data[ENC_NONCE_SIZE_BYTES] = { 0 };

	void SetNull();
	bool IsValid() const;

	void Generate(); ///< Generate a new random nonce

	bool SetFromHexString(const string& str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);

	bool operator == (const DS_EncNonce &b) const {
		return (sodium_memcmp(data, b.data, ENC_NONCE_SIZE_BYTES) == 0);
	}
	bool operator != (const DS_EncNonce &b) const {
		return (sodium_memcmp(data, b.data, ENC_NONCE_SIZE_BYTES) != 0);
	}
	bool operator < (const DS_EncNonce &b) const {
		if (memcmp(data, b.data, ENC_NONCE_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

/**
 * Short nonce gives you a way to encrypt and decrypt data while only having to send a shorter nonce after that.
 * You still need to call Generate() to make a full nonce and give it to the other side once, but after that you can use the Increment/GetIncrement functions to only send a smaller amount of data with each packet.
 * I wouldn't use any less than a uint32 for this.
 */
template <class ENC_NONCE_INT_TYPE>
class DSL_SODIUM_API_CLASS DS_EncShortNonce: public DS_EncNonce {
public:
	ENC_NONCE_INT_TYPE GetIncrement() const {
		return *((ENC_NONCE_INT_TYPE *)&data[0]);
	}
	void SetIncrement(ENC_NONCE_INT_TYPE x) {
		*((ENC_NONCE_INT_TYPE *)&data[0]) = x;
	}
	void Increment() {
		randombytes_buf(&data[0], sizeof(ENC_NONCE_INT_TYPE));
	}
	void IncrementByOne() {
		sodium_increment(&data[0], sizeof(ENC_NONCE_INT_TYPE));
	}
};

class DS_EncPubKey;
class DS_EncPrivKey;

/**
 * If you are going to be encrypting/decrypting with the same key pair a lot you can use this to have the shared key pre-computed to improve performance.
 * See https://libsodium.gitbook.io/doc/public-key_cryptography/authenticated_encryption#precalculation-interface for more information.
 */
class DSL_SODIUM_API_CLASS DS_EncSharedKey {
private:
	bool fLocked;
	void checkLocking(bool fForce = false);

public:
	uint8_t data[ENC_SHARED_KEY_BYTE_SIZE];

	DS_EncSharedKey();
	~DS_EncSharedKey();

	void SetNull();
	bool IsValid() const;

	bool Calculate(DS_EncPrivKey& privkey, DS_EncPubKey& pubkey);
	bool Encrypt(DS_EncNonce& nonce, DSL_BUFFER * buf);
	bool Decrypt(DS_EncNonce& nonce, DSL_BUFFER * buf);

	bool operator == (const DS_EncSharedKey &b) const {
		return (sodium_memcmp(data, b.data, ENC_SHARED_KEY_BYTE_SIZE) == 0);
	}
	bool operator != (const DS_EncSharedKey &b) const {
		return (sodium_memcmp(data, b.data, ENC_SHARED_KEY_BYTE_SIZE) != 0);
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

	string GetString() const { return bin2hex(key, sizeof(key)); }
	void SetNull();
	bool IsValid() const;

	bool SetFromHexString(const string& str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);

	bool operator == (const DS_EncPubKey &b) const {
		return (sodium_memcmp(key, b.key, ENC_PUBKEY_SIZE_BYTES) == 0);
	}
	bool operator != (const DS_EncPubKey &b) const {
		return (sodium_memcmp(key, b.key, ENC_PUBKEY_SIZE_BYTES) != 0);
	}
	bool operator < (const DS_EncPubKey &b) const {
		if (memcmp(key, b.key, ENC_PUBKEY_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

/**
 * A private key for encrypting & decrypting data.
 * This is a wrapper for libsodium's public-key authenticated encryption API (crypto_box_*).
 * Key exchange: X25519
 * Encryption: XSalsa20
 * Authentication: Poly1305
 */
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

	string GetString() const { return bin2hex(key, sizeof(key)); }
	void SetNull();
	bool IsValid() const;

	bool Encrypt(const DS_EncPubKey& recpt_key, const DS_EncNonce& nonce, DSL_BUFFER * buf);
	bool Decrypt(const DS_EncPubKey& sender_key, const DS_EncNonce& nonce, DSL_BUFFER * buf);
	bool Generate();

	bool SetFromHexString(const string& str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);
	bool SetBothFromBinaryData(const uint8_t * sdata, size_t slen, const uint8_t * pdata, size_t plen);

	bool operator == (const DS_EncPrivKey &b) const {
		return (sodium_memcmp(key, b.key, ENC_PRIVKEY_SIZE_BYTES) == 0);
	}
	bool operator != (const DS_EncPrivKey &b) const {
		return (sodium_memcmp(key, b.key, ENC_PRIVKEY_SIZE_BYTES) != 0);
	}
	bool operator < (const DS_EncPrivKey &b) const {
		if (memcmp(key, b.key, ENC_PRIVKEY_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

/**@}*/


#endif // __DSL_SODIUM_KEYS_ENCRYPTION__
