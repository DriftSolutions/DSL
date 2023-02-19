//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef ENABLE_OPENSSL
#include <drift/dslcore.h>
#include <drift/sockets3.h>
#include <drift/openssl.h>

DSL_Mutex  * sslSockMutex()
{
	static DSL_Mutex actualMutex;
	return &actualMutex;
}
//#define OVERPROTECTIVE_OPENSSL

extern DSL_Library_Registerer dsl_openssl_autoreg;
DSL_Sockets3_OpenSSL::DSL_Sockets3_OpenSSL() {
	dsl_openssl_autoreg.EnsureLinked();
	ctx = NULL;
	avail_flags |= DS3_FLAG_SSL;
}

DSL_Sockets3_OpenSSL::~DSL_Sockets3_OpenSSL() {
	LockMutexPtr(sslSockMutex());
	if (ctx) {
		SSL_CTX_free(ctx);
		ctx = NULL;
	}
	RelMutexPtr(sslSockMutex());
}

DSL_SOCKET * DSL_Sockets3_OpenSSL::pAllocSocket() {
	return new DSL_SOCKET_OPENSSL();
}

int DSL_Sockets3_OpenSSL::pRecv(DSL_SOCKET * pSock, char * buf, uint32 bufsize) {
	DSL_SOCKET_OPENSSL * sock = static_cast<DSL_SOCKET_OPENSSL *>(pSock);

	if (sock->ssl) {
#if defined(OVERPROTECTIVE_OPENSSL)
		AutoMutex(sslSockMutex);
#endif
		int n = SSL_read(sock->ssl, buf, bufsize);
		if (n <= 0) {
			sprintf(bError, "Error returned by SSL_read(): %d", n);
			bErrNo = 0x54530014;
			ERR_print_errors_fp(stderr);
			return (n == 0) ? 0 : -1;
		}
		return n;
	}
	return DSL_Sockets3_Base::pRecv(sock, buf, bufsize);
}

int DSL_Sockets3_OpenSSL::pPeek(DSL_SOCKET * pSock, char * buf, uint32 bufsize) {
	DSL_SOCKET_OPENSSL * sock = static_cast<DSL_SOCKET_OPENSSL *>(pSock);

	if (sock->ssl) {
#if defined(OVERPROTECTIVE_OPENSSL)
		AutoMutex(sslSockMutex);
#endif
		int n = SSL_peek(sock->ssl, buf, bufsize);
		if (n >= 0) { return n; }
		n = SSL_get_error(sock->ssl, n);
		//sprintf(bError, "Error returned by SSL_peek(): %d", n);
		memset(bError, 0, sizeof(bError));
		ERR_error_string_n(n, bError, sizeof(bError));
		bErrNo = 0x54530014;
		ERR_print_errors_fp(stderr);
		return -1;
	}
	return DSL_Sockets3_Base::pPeek(sock, buf, bufsize);
}


int DSL_Sockets3_OpenSSL::pSend(DSL_SOCKET * pSock, const char * data, uint32 datalen) {
	DSL_SOCKET_OPENSSL * sock = static_cast<DSL_SOCKET_OPENSSL *>(pSock);

	if (sock->ssl) {
#if defined(OVERPROTECTIVE_OPENSSL)
		LockMutexPtr(sslSockMutex());
#endif
#if defined(WIN32)
		int n = 0;
		__try {
			n = SSL_write(sock->ssl, data, datalen);
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			n = -1;
		}
#else
		int n = SSL_write(sock->ssl, data, datalen);
#endif
#if defined(OVERPROTECTIVE_OPENSSL)
		RelMutexPtr(sslSockMutex());
#endif
		if (n <= 0) {
			sprintf(bError, "Error returned by SSL_write(): %d", n);
			bErrNo = 0x54530020;
			printf("OpenSSL Error: %s\n", ERR_error_string(ERR_get_error(), bError));
			//ERR_print_errors_fp(stderr);
			return -1;
		}
		return n;
	}

	return DSL_Sockets3_Base::pSend(sock, data, datalen);
}

bool DSL_Sockets3_OpenSSL::EnableSSL(const char * cert, DS3_SSL_METHOD method) {
	AutoMutexPtr(sslSockMutex());
	const SSL_METHOD * meth = NULL;
	switch (method) {
#if defined(DTLS1_VERSION) && !defined(NO_DTLS1)
		case DS3_SSL_METHOD_DTLS1:
#if OPENSSL_VERSION_NUMBER >= 0x30000000
			meth = DTLS_method();
#else
			meth = DTLSv1_method();
#endif
			break;
#endif

#if OPENSSL_VERSION_NUMBER >= 0x30000000
		default:
			meth = TLS_method();
			break;
#else

#if OPENSSL_VERSION_NUMBER > 0x10000080
#ifndef OPENSSL_NO_TLS1_METHOD
		case DS3_SSL_METHOD_TLS1_0:
			meth = TLSv1_method();
			break;
#endif
#ifndef OPENSSL_NO_TLS1_1_METHOD
		case DS3_SSL_METHOD_TLS1_1:
			meth = TLSv1_1_method();
			break;
#endif
#ifndef OPENSSL_NO_TLS1_2_METHOD
		case DS3_SSL_METHOD_TLS1_2:
			meth = TLSv1_2_method();
			break;
#endif
#endif

		default:
			if (!this->silent) {
				printf("DSL_Sockets3: Unknown TLS method, using default...\n");
			}
		case DS3_SSL_METHOD_TLS:
#if OPENSSL_VERSION_NUMBER > 0x10100000
			meth = TLS_method();
#elif OPENSSL_VERSION_NUMBER > 0x10000080
			meth = TLSv1_2_method();
#else
			meth = TLSv1_method();
#endif
			break;
#endif // OPENSSL_VERSION_NUMBER >= 0x30000000
	}

	ctx = SSL_CTX_new((SSL_METHOD *)meth);
	if (!ctx) {
		//printf("OpenSSL Error: %u, %s\n", ERR_get_error(), ERR_error_string(ERR_get_error(), bError));
		ERR_print_errors_fp(stderr);
		strcpy(bError, "Error initializing a SSL_CTX structure!");
		bErrNo = 0x54530001;
		return false;
	}
	SSL_CTX_set_mode(ctx, SSL_CTX_get_mode(ctx) | SSL_MODE_AUTO_RETRY);
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3 | SSL_OP_NO_SSLv2);

	if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		strcpy(bError, "Error loading your certificate!");
		bErrNo = 0x54530002;
		SSL_CTX_free(ctx);
		ctx = NULL;
		return false;
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		strcpy(bError, "Error loading your Private Key!");
		bErrNo = 0x54530003;
		SSL_CTX_free(ctx);
		ctx = NULL;
		return false;
	}

	if (!SSL_CTX_check_private_key(ctx)) {
		ERR_print_errors_fp(stderr);
		strcpy(bError, "Private key does not match the certificate public key!");
		bErrNo = 0x54530004;
		SSL_CTX_free(ctx);
		ctx = NULL;
		return false;
	}

	enabled_flags |= DS3_FLAG_SSL;
	return true;
}

bool DSL_Sockets3_OpenSSL::SwitchToSSL_Server(DSL_SOCKET * pSock) {
	DSL_SOCKET_OPENSSL * sock = static_cast<DSL_SOCKET_OPENSSL *>(pSock);

#if defined(OVERPROTECTIVE_OPENSSL)
	AutoMutex(sslSockMutex);
#endif
	if (!sock->ssl) {
		BIO * sBio = BIO_new_socket(sock->sock, 0); // SSL_free() will free this for us later
		if (!sBio) {
			sprintf(bError, "Error Allocating a BIO!\n");
			bErrNo = 0x54530010;
			return false;
		}

		sock->ssl = SSL_new(ctx);
		if (!sock->ssl) {
			sprintf(bError, "Error Allocating a SSL!\n");
			bErrNo = 0x54530011;
			BIO_free(sBio);
			return false;
		}
		SSL_set_bio(sock->ssl, sBio, sBio);
}
	if (sock->ssl) {
		int iret = SSL_accept(sock->ssl);
		switch (iret) {
			case 1: {
				//					char buf[1024];
				if (!silent) {
					printf("SSL connection established\n");
					printf("SSL Version: %s, Cipher: %s\n", SSL_get_version(sock->ssl), SSL_get_cipher(sock->ssl));
				}
				//printf("SSL info: %s\n",SSL_CIPHER_description(sock->ssl->session->cipher, buf, sizeof(buf)));
				int max_bits = 0;
				int bits = SSL_get_cipher_bits(sock->ssl, &max_bits);
				if (!silent) { printf("Using %d of %d maximum possible bits for security\n", bits, max_bits); }
				sock->flags |= DS3_FLAG_SSL;
				return true;
				break;
			}
			case 0:
				sprintf(bError, "SSL connection failed cleanly: %d", SSL_get_error(sock->ssl, iret));
				bErrNo = 0x54530012;
				printf("OpenSSL Error: %d, %s\n", iret, ERR_error_string(ERR_get_error(), bError));
				//ERR_print_errors_fp(stderr);
				break;
			default:
				sprintf(bError, "SSL connection failed");
				bErrNo = 0x54530013;
				//ERR_print_errors_fp(stderr);
				printf("OpenSSL Error: %d, %s\n", iret, ERR_error_string(ERR_get_error(), bError));
				break;
		}
	}

	return false;
}
bool DSL_Sockets3_OpenSSL::SwitchToSSL_Client(DSL_SOCKET * pSock) {
	DSL_SOCKET_OPENSSL * sock = static_cast<DSL_SOCKET_OPENSSL *>(pSock);

#if defined(OVERPROTECTIVE_OPENSSL)
	AutoMutex(sslSockMutex);
#endif
	if (!sock->ssl) {
		BIO * sBio = BIO_new_socket(sock->sock, 0); // SSL_free() will free this for us later
		if (!sBio) {
			sprintf(bError, "Error Allocating a BIO!\n");
			bErrNo = 0x54530010;
			return false;
		}

		sock->ssl = SSL_new(ctx);
		if (!sock->ssl) {
			sprintf(bError, "Error Allocating a SSL!\n");
			bErrNo = 0x54530011;
			BIO_free(sBio);
			return false;
		}

		SSL_set_bio(sock->ssl, sBio, sBio);
	}

	if (sock->ssl) {
		int iret = SSL_connect(sock->ssl);

		switch (iret) {
			case 1: {
				//					char buf[1024];
				if (!silent) {
					printf("SSL connection established\n");
					printf("SSL Version: %s, Cipher: %s\n", SSL_get_version(sock->ssl), SSL_get_cipher(sock->ssl));
				}
				//printf("SSL info: %s\n",SSL_CIPHER_description(sock->ssl->session->cipher, buf, sizeof(buf)));
				int max_bits = 0;
				int bits = SSL_get_cipher_bits(sock->ssl, &max_bits);
				if (!silent) { printf("Using %d of %d maximum possible bits for security\n", bits, max_bits); }
				sock->flags |= DS3_FLAG_SSL;
				return true;
				break;
			}
			case 0:
				sprintf(bError, "SSL connection failed cleanly: %d", SSL_get_error(sock->ssl, iret));
				bErrNo = 0x54530012;
				ERR_print_errors_fp(stderr);
				break;
			default:
				sprintf(bError, "SSL connection failed");
				bErrNo = 0x54530013;
				printf("OpenSSL Error: %s\n", ERR_error_string(ERR_get_error(), bError));
				//ERR_print_errors_fp(stderr);
				break;
		}
	}

	return false;
}

int DSL_Sockets3_OpenSSL::pSelect_Read(DSL_SOCKET * pSock, timeval * timeo) {
	DSL_SOCKET_OPENSSL * sock = static_cast<DSL_SOCKET_OPENSSL *>(pSock);

	if (sock->ssl) {
#if defined(OVERPROTECTIVE_OPENSSL)
		AutoMutex(sslSockMutex);
#endif
		int n = SSL_pending(sock->ssl);
		if (n > 0) {
			return 1;
		}
	}

	int ret = DSL_Sockets3_Base::pSelect_Read(sock, timeo);

	if (sock->ssl && ret == 1) {
		char buf[16];
		int n = Peek(sock, buf, 1);
		if (n >= 0) { return 1; }
		return n;
	}

	return ret;
}

void DSL_Sockets3_OpenSSL::pCloseSSL(DSL_SOCKET * pSock) {
	DSL_SOCKET_OPENSSL * sock = static_cast<DSL_SOCKET_OPENSSL *>(pSock);

	if (sock->ssl) {
#if defined(OVERPROTECTIVE_OPENSSL)
		LockMutexPtr(sslSockMutex());
#endif
		if (!SSL_shutdown(sock->ssl)) {
			SSL_shutdown(sock->ssl); // will wait for a proper shutdown, or socket error
		}
		SSL_free(sock->ssl);
#if defined(OVERPROTECTIVE_OPENSSL)
		RelMutexPtr(sslSockMutex());
#endif
	}
}

X509 * DSL_Sockets3_OpenSSL::GetSSL_Cert(DSL_SOCKET * pSock) { /// Don't forget to free with X509_free
	DSL_SOCKET_OPENSSL * sock = static_cast<DSL_SOCKET_OPENSSL *>(pSock);

	X509 * ret = NULL;
#if defined(OVERPROTECTIVE_OPENSSL)
	AutoMutex(sslSockMutex);
#endif
	if (sock->ssl) {
#if OPENSSL_VERSION_NUMBER >= 0x30000000
		ret = SSL_get1_peer_certificate(sock->ssl);		
#else
		ret = SSL_get_peer_certificate(sock->ssl);
#endif
	}
	return ret;
}

SSL_CTX * DSL_Sockets3_OpenSSL::GetSSL_CTX() {
	return ctx;
}

#endif // ENABLE_OPENSSL
