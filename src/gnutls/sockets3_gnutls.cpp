//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef ENABLE_GNUTLS
#include <drift/dslcore.h>
#include <drift/sockets3.h>
#include <drift/gnutls.h>

DSL_Mutex  * gtlsSockMutex()
{
	static DSL_Mutex actualMutex;
	return &actualMutex;
}

extern DSL_Library_Registerer dsl_gnutls_autoreg;
DSL_Sockets3_GnuTLS::DSL_Sockets3_GnuTLS() {
	gnutls_cred = NULL;
	avail_flags |= DS3_FLAG_SSL;
	dsl_gnutls_autoreg.EnsureLinked();
}

DSL_Sockets3_GnuTLS::~DSL_Sockets3_GnuTLS() {
	LockMutexPtr(gtlsSockMutex());
	if (gnutls_cred != NULL) {
		gnutls_certificate_free_credentials(gnutls_cred);
		gnutls_cred = NULL;
	}
	RelMutexPtr(gtlsSockMutex());
}

DSL_SOCKET * DSL_Sockets3_GnuTLS::pAllocSocket() {
	return new DSL_SOCKET_GNUTLS();
}

int DSL_Sockets3_GnuTLS::pRecv(DSL_SOCKET * pSock, char * buf, uint32 bufsize) {
	DSL_SOCKET_GNUTLS * sock = static_cast<DSL_SOCKET_GNUTLS *>(pSock);

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

	return DSL_Sockets3_Base::pRecv(sock, buf, bufsize);
}

int DSL_Sockets3_GnuTLS::pPeek(DSL_SOCKET * pSock, char * buf, uint32 bufsize) {
	DSL_SOCKET_GNUTLS * sock = static_cast<DSL_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		ssize_t n = gnutls_record_check_pending(sock->gtls);
		if (n > 0) { return n; }
	}

	return DSL_Sockets3_Base::pPeek(sock, buf, bufsize);
}


int DSL_Sockets3_GnuTLS::pSend(DSL_SOCKET * pSock, const char * data, uint32 datalen) {
	DSL_SOCKET_GNUTLS * sock = static_cast<DSL_SOCKET_GNUTLS *>(pSock);

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

	return DSL_Sockets3_Base::pSend(sock, data, datalen);
}

bool DSL_Sockets3_GnuTLS::EnableSSL(const char * cert, DS3_SSL_METHOD method) {
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

	enabled_flags |= DS3_FLAG_SSL;
	return true;
}

bool DSL_Sockets3_GnuTLS::SwitchToSSL_Server(DSL_SOCKET * pSock) {
	DSL_SOCKET_GNUTLS * sock = static_cast<DSL_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
		sock->flags &= ~DS3_FLAG_SSL;
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

	sock->flags |= DS3_FLAG_SSL;
	sock->ssl_is_client = false;
	return true;
}

bool DSL_Sockets3_GnuTLS::SwitchToSSL_Client(DSL_SOCKET * pSock) {
	DSL_SOCKET_GNUTLS * sock = static_cast<DSL_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
		sock->flags &= ~DS3_FLAG_SSL;
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

	sock->flags |= DS3_FLAG_SSL;
	sock->ssl_is_client = true;
	return true;
}

int DSL_Sockets3_GnuTLS::pSelect_Read(DSL_SOCKET * pSock, timeval * timeo) {
	DSL_SOCKET_GNUTLS * sock = static_cast<DSL_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		int n = gnutls_record_check_pending(sock->gtls);
		if (n > 0) {
			return 1;
		}
	}

	return DSL_Sockets3_Base::pSelect_Read(sock, timeo);
}

void DSL_Sockets3_GnuTLS::pCloseSSL(DSL_SOCKET * pSock) {
	DSL_SOCKET_GNUTLS * sock = static_cast<DSL_SOCKET_GNUTLS *>(pSock);

	if (sock->gtls) {
		int n = gnutls_bye(sock->gtls, GNUTLS_SHUT_WR);
		if (n != GNUTLS_E_SUCCESS) {
			snprintf(bError, sizeof(bError), "Error ending SSL/TLS session! (%s)", gnutls_strerror(n));
		}
		gnutls_deinit(sock->gtls);
		sock->gtls = NULL;
	}
}

gnutls_session_t DSL_Sockets3_GnuTLS::GetSSL_CTX() {
	return ctx;
}

#endif // ENABLE_GNUTLS
