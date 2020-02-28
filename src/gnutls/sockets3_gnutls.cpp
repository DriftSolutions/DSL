//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifdef ENABLE_GNUTLS
#include <drift/dslcore.h>
#include <drift/sockets3.h>
#include <drift/gnutls.h>

Titus_Mutex  * gtlsSockMutex()
{
	static Titus_Mutex actualMutex;
	return &actualMutex;
}

extern DSL_Library_Registerer dsl_gnutls_autoreg;
Titus_Sockets3_GnuTLS::Titus_Sockets3_GnuTLS() {
	gnutls_cred = NULL;
	avail_flags |= TS3_FLAG_SSL;
	dsl_gnutls_autoreg.EnsureLinked();
}

Titus_Sockets3_GnuTLS::~Titus_Sockets3_GnuTLS() {
	LockMutexPtr(gtlsSockMutex());
	if (gnutls_cred != NULL) {
		gnutls_certificate_free_credentials(gnutls_cred);
		gnutls_cred = NULL;
	}
	RelMutexPtr(gtlsSockMutex());
}

TITUS_SOCKET * Titus_Sockets3_GnuTLS::pAllocSocket() {
	return new TITUS_SOCKET_GNUTLS();
}

int Titus_Sockets3_GnuTLS::pRecv(TITUS_SOCKET * pSock, char * buf, uint32 bufsize) {
	TITUS_SOCKET_GNUTLS * sock = static_cast<TITUS_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		ssize_t n;
		while ((n = gnutls_record_recv(sock->gtls, buf, bufsize)) == GNUTLS_E_INTERRUPTED || n == GNUTLS_E_AGAIN || n == GNUTLS_E_REHANDSHAKE) {
			if (n == GNUTLS_E_REHANDSHAKE) {
				//rehandshake
				if (sock->ssl_is_client) {
					if (!SwitchToSSL_Client(sock)) {
						return -1;
					}
				} else {
					if (!SwitchToSSL_Server(sock)) {
						return -1;
					}
				}
			}
		}
		if (n <= 0) {
			snprintf(bError, sizeof(bError), "Error returned by gnutls_record_recv(): %d -> %s", n, gnutls_strerror(n));
			bErrNo = 0x54530020;
			return (n == 0) ? 0:-1;
		}
		return n;
	}

	return Titus_Sockets3_Base::pRecv(sock, buf, bufsize);
}

int Titus_Sockets3_GnuTLS::pPeek(TITUS_SOCKET * pSock, char * buf, uint32 bufsize) {
	TITUS_SOCKET_GNUTLS * sock = static_cast<TITUS_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		ssize_t n = gnutls_record_check_pending(sock->gtls);
		if (n > 0) { return n; }
	}

	return Titus_Sockets3_Base::pPeek(sock, buf, bufsize);
}


int Titus_Sockets3_GnuTLS::pSend(TITUS_SOCKET * pSock, const char * data, uint32 datalen) {
	TITUS_SOCKET_GNUTLS * sock = static_cast<TITUS_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		ssize_t n;
		while ((n = gnutls_record_send(sock->gtls, data, datalen)) == GNUTLS_E_INTERRUPTED || n == GNUTLS_E_AGAIN) {}
		if (n <= 0) {
			snprintf(bError, sizeof(bError), "Error returned by gnutls_record_send(): %d -> %s", n, gnutls_strerror(n));
			bErrNo = 0x54530020;
			return -1;
		}
		return n;
	}

	return Titus_Sockets3_Base::pSend(sock, data, datalen);
}

bool Titus_Sockets3_GnuTLS::EnableSSL(const char * cert, TS3_SSL_METHOD method) {
	AutoMutexPtr(gtlsSockMutex());
	if (gnutls_cred != NULL) {
		gnutls_certificate_free_credentials(gnutls_cred);
		gnutls_cred = NULL;
	}
	int n = gnutls_certificate_allocate_credentials(&gnutls_cred);
	if (n != GNUTLS_E_SUCCESS) {
		snprintf(bError,sizeof(bError),"Error allocating credentials structure! (%s)", gnutls_strerror(n));
		bErrNo = 0x54530002;
		return false;
	}
	n = gnutls_certificate_set_x509_key_file(gnutls_cred, cert, cert, GNUTLS_X509_FMT_PEM);
	if (n != GNUTLS_E_SUCCESS) {
		snprintf(bError,sizeof(bError),"Error loading your certificate! (%s)", gnutls_strerror(n));
		bErrNo = 0x54530002;
		return false;
	}

	enabled_flags |= TS3_FLAG_SSL;
	return true;
}

bool Titus_Sockets3_GnuTLS::SwitchToSSL_Server(TITUS_SOCKET * pSock) {
	TITUS_SOCKET_GNUTLS * sock = static_cast<TITUS_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
		sock->flags &= ~TS3_FLAG_SSL;
	}

	int n = gnutls_init(&sock->gtls, GNUTLS_SERVER);
	if (n != GNUTLS_E_SUCCESS) {
		snprintf(bError,sizeof(bError),"Error allocating TLS session! (%s)", gnutls_strerror(n));
		bErrNo = 0x54530010;
		return false;
	}
	n = gnutls_credentials_set(sock->gtls, GNUTLS_CRD_CERTIFICATE, gnutls_cred);
	if (n != GNUTLS_E_SUCCESS) {
		snprintf(bError,sizeof(bError),"Error associating your certificate! (%s)", gnutls_strerror(n));
		bErrNo = 0x54530010;
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
		return false;
	}
	gnutls_transport_set_ptr2(sock->gtls, (gnutls_transport_ptr_t)sock->sock, (gnutls_transport_ptr_t)sock->sock);
#if GNUTLS_VERSION_NUMBER > 0x030100
	gnutls_handshake_set_timeout(sock->gtls, 10000);
#endif
	const char *p=NULL;
	gnutls_priority_set_direct(sock->gtls, "NORMAL", &p);
	while ((n = gnutls_handshake(sock->gtls)) != GNUTLS_E_SUCCESS && !gnutls_error_is_fatal(n)) {}
	if (n != GNUTLS_E_SUCCESS) {
		snprintf(bError,sizeof(bError),"Error performing SSL/TLS handshake! (%s)", gnutls_strerror(n));
		bErrNo = 0x54530010;
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
		return false;
	}

	sock->flags |= TS3_FLAG_SSL;
	sock->ssl_is_client = false;
	return true;
}

bool Titus_Sockets3_GnuTLS::SwitchToSSL_Client(TITUS_SOCKET * pSock) {
	TITUS_SOCKET_GNUTLS * sock = static_cast<TITUS_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
		sock->flags &= ~TS3_FLAG_SSL;
	}

	int n = gnutls_init(&sock->gtls, GNUTLS_CLIENT);
	if (n != GNUTLS_E_SUCCESS) {
		snprintf(bError,sizeof(bError),"Error allocating TLS session! (%s)", gnutls_strerror(n));
		bErrNo = 0x54530010;
		return false;
	}
	n = gnutls_credentials_set(sock->gtls, GNUTLS_CRD_CERTIFICATE, gnutls_cred);
	if (n != GNUTLS_E_SUCCESS) {
		snprintf(bError,sizeof(bError),"Error associating your certificate! (%s)", gnutls_strerror(n));
		bErrNo = 0x54530010;
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
		return false;
	}
	gnutls_transport_set_ptr2(sock->gtls, (gnutls_transport_ptr_t)sock->sock, (gnutls_transport_ptr_t)sock->sock);
#if GNUTLS_VERSION_NUMBER > 0x030100
	gnutls_handshake_set_timeout(sock->gtls, 10000);
#endif
	const char *p=NULL;
	gnutls_priority_set_direct(sock->gtls, "NORMAL", &p);
	while ((n = gnutls_handshake(sock->gtls)) != GNUTLS_E_SUCCESS && !gnutls_error_is_fatal(n)) {}
	if (n != GNUTLS_E_SUCCESS) {
		snprintf(bError,sizeof(bError),"Error performing SSL/TLS handshake! (%s)", gnutls_strerror(n));
		bErrNo = 0x54530010;
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
		return false;
	}

	sock->flags |= TS3_FLAG_SSL;
	sock->ssl_is_client = true;
	return true;
}

int Titus_Sockets3_GnuTLS::pSelect_Read(TITUS_SOCKET * pSock, timeval * timeo) {
	TITUS_SOCKET_GNUTLS * sock = static_cast<TITUS_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		int n = gnutls_record_check_pending(sock->gtls);
		if (n > 0) {
			return 1;
		}
	}

	return Titus_Sockets3_Base::pSelect_Read(sock, timeo);
}

void Titus_Sockets3_GnuTLS::pCloseSSL(TITUS_SOCKET * pSock) {
	TITUS_SOCKET_GNUTLS * sock = static_cast<TITUS_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		int n = gnutls_bye(sock->gtls, GNUTLS_SHUT_WR);
		if (n != GNUTLS_E_SUCCESS) {
			snprintf(bError, sizeof(bError), "Error ending SSL/TLS session! (%s)", gnutls_strerror(n));
		}
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
	}
}

gnutls_session_t Titus_Sockets3_GnuTLS::GetSSL_CTX() {
	return ctx;
}

#endif // ENABLE_GNUTLS
