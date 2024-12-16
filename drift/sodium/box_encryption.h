//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_SODIUM_BOX_ENCRYPTION__
#define __DSL_SODIUM_BOX_ENCRYPTION__

/** \addtogroup sodium
 * @{
 */

#define BOX_PRIVKEY_SIZE_BYTES (crypto_secretbox_KEYBYTES)
#define BOX_NONCE_SIZE_BYTES crypto_secretbox_NONCEBYTES

/**
 * A nonce used for encrypting & decrypting data with DS_BoxPrivKey.
 */
class DSL_SODIUM_API_CLASS DS_BoxNonce {
public:
	uint8_t data[BOX_NONCE_SIZE_BYTES] = { 0 };

	string GetString() const { return bin2hex(data, sizeof(data)); }
	void SetNull();
	bool IsValid() const;

	void Generate(); ///< Generate a new random nonce

	bool SetFromHexString(const string& str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);

	bool operator == (const DS_EncNonce &b) const {
		return (sodium_memcmp(data, b.data, BOX_NONCE_SIZE_BYTES) == 0);
	}
	bool operator != (const DS_EncNonce &b) const {
		return (sodium_memcmp(data, b.data, BOX_NONCE_SIZE_BYTES) != 0);
	}
	bool operator < (const DS_BoxNonce &b) const {
		if (memcmp(data, b.data, BOX_NONCE_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

/**
 * A private key for encrypting & decrypting data.
 * This is a wrapper for libsodium's secret box authenticated encryption API (crypto_secretbox_*).
 * Encryption: XSalsa20
 * Authentication: Poly1305
 */
class DSL_SODIUM_API_CLASS DS_BoxPrivKey {
private:
	bool fLocked;
	void checkLocking(bool fForce = false);

public:
	uint8_t key[BOX_PRIVKEY_SIZE_BYTES];

	DS_BoxPrivKey();
	~DS_BoxPrivKey();

	string GetString() const { return bin2hex(key, sizeof(key)); }
	void SetNull();
	bool IsValid() const;

	bool Encrypt(const DS_BoxNonce& nonce, DSL_BUFFER * buf);
	bool Decrypt(const DS_BoxNonce& nonce, DSL_BUFFER * buf);
	bool Generate();

	bool SetFromHexString(const string& str);
	bool SetFromBinaryData(const uint8_t * pdata, size_t len);

	bool operator == (const DS_EncPrivKey &b) const {
		return (sodium_memcmp(key, b.key, BOX_PRIVKEY_SIZE_BYTES) == 0);
	}
	bool operator != (const DS_EncPrivKey &b) const {
		return (sodium_memcmp(key, b.key, BOX_PRIVKEY_SIZE_BYTES) != 0);
	}
	bool operator < (const DS_EncPrivKey &b) const {
		if (memcmp(key, b.key, BOX_PRIVKEY_SIZE_BYTES) < 0) {
			return true;
		}
		return false;
	}
};

/**@}*/


#endif // __DSL_SODIUM_KEYS_ENCRYPTION__
