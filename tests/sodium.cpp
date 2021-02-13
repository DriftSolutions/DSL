//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dsl.h>

int main(int argc, char * argv[]) {
	if (!dsl_init()) {
		printf("dsl_init() failed!\n");
		return 1;
	}

	string str = "Jen Towns is #1";
	DS_Hash hash = DS_Hasher().HashData((const uint8_t *)str.c_str(), str.length());

	string correct = "457814f56ef15896dc58495609f747e7836229bc71136e92fe9fbc8c59aa142a";
	string hashstr = hash.GetString();
	if (strcmp(correct.c_str(), hashstr.c_str()) == 0) {
		printf("[sodium] blake2b success!\n");
	} else {
		printf("[sodium] blake2b error! Got %s, should be %s\n", hashstr.c_str(), correct.c_str());
	}

	DS_EncPrivKey enckey;
	DS_EncPrivKey recvkey;
	if (enckey.Generate() && recvkey.Generate()) {
		DS_EncNonce nonce;
		nonce.Generate();
		DSL_BUFFER buf;
		buffer_init(&buf);
		buffer_set(&buf, str.c_str(), str.length() + 1);
		if (enckey.Encrypt(recvkey.pubkey, nonce, &buf)) {
			if (recvkey.Decrypt(enckey.pubkey, nonce, &buf)) {
				if (strcmp(str.c_str(), buf.data) == 0) {
					printf("[sodium] Encryption success!\n");
				} else {
					printf("[sodium] Decrypted data does not match!\n");
				}
			} else {
				printf("[sodium] Error decrypting data!\n");
			}
		} else {
			printf("[sodium] Error encrypting data!\n");
		}
		buffer_free(&buf);
	} else {
		printf("[sodium] Error generating encryption keys!\n");
	}

	string testkey = "7bb8b38c16ebe13a20a547bc3ef75e3879c156529bab1c427038d508ba5cb2bf1290a06ce64b53d42af8c3ecff1e503bd68e040179163d93e4c1e4260bc412fd";
	string correctsig = "99a9ad3e7ce8c56153715e34ef6fa56b132dc2255b148b603c973feb56b5558154d0300ba262fa7a67b73cd3657ac9b2a7d5cce13348e3d7b7356ad78ad84207";

	DS_SigPrivKey sigkey;
	if (sigkey.SetFromHexString(testkey)) {
		DS_Signature sig;
		if (sig.SignData(sigkey, (const uint8_t *)str.c_str(), str.length())) {
			if (stricmp(sig.GetString().c_str(), correctsig.c_str()) == 0) {
				printf("[sodium] Signing success!\n");
			} else {
				printf("[sodium]: Signature error! Got %s, should be %s\n", sig.GetString().c_str(), correctsig.c_str());
			}
		} else {
			printf("[sodium] Error signing data!\n");
		}
	} else {
		printf("[sodium] Error setting signing keys!\n");
	}

	dsl_cleanup();
	return 0;
}
