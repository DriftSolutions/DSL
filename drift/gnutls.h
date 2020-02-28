//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef __DRIFT_GNUTLS_H__
#define __DRIFT_GNUTLS_H__

#include <drift/sockets3.h>

#if defined(DSL_DLL) && defined(WIN32)
	#if defined(DSL_GNUTLS_EXPORTS)
		#define DSL_GNUTLS_API extern "C" __declspec(dllexport)
		#define DSL_GNUTLS_API_CLASS __declspec(dllexport)
	#else
		#define DSL_GNUTLS_API extern "C" __declspec(dllimport)
		#define DSL_GNUTLS_API_CLASS __declspec(dllimport)
	#endif
#else
	#define DSL_GNUTLS_API
	#define DSL_GNUTLS_API_CLASS
#endif

class DSL_GNUTLS_API_CLASS TITUS_SOCKET_GNUTLS: public TITUS_SOCKET {
public:
	gnutls_session_t gtls = NULL;
	bool ssl_is_client = false;
};

class DSL_GNUTLS_API_CLASS Titus_Sockets3_GnuTLS: public Titus_Sockets3_SSL {
	protected:
		virtual TITUS_SOCKET * pAllocSocket();
		virtual int pRecv(TITUS_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int pPeek(TITUS_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int pSend(TITUS_SOCKET * sock, const char * buf, uint32 bufsize);
		virtual int pSelect_Read(TITUS_SOCKET * sock, timeval * timeo);
		virtual void pCloseSSL(TITUS_SOCKET * sock);
	private:
		gnutls_session_t ctx = NULL;
		gnutls_certificate_credentials_t gnutls_cred;
	public:
		Titus_Sockets3_GnuTLS();
		virtual ~Titus_Sockets3_GnuTLS();

		virtual bool EnableSSL(const char * cert_fn, TS3_SSL_METHOD method);
		virtual bool SwitchToSSL_Server(TITUS_SOCKET * sock);
		virtual bool SwitchToSSL_Client(TITUS_SOCKET * sock);

		virtual gnutls_session_t GetSSL_CTX();
};

#endif // __DRIFT_GNUTLS_H__
