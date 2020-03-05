//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef __DRIFT_OPENSSL_H__
#define __DRIFT_OPENSSL_H__

#include <drift/sockets3.h>

#if defined(DSL_DLL) && defined(WIN32)
	#if defined(DSL_OPENSSL_EXPORTS)
		#define DSL_OPENSSL_API extern "C" __declspec(dllexport)
		#define DSL_OPENSSL_API_CLASS __declspec(dllexport)
	#else
		#define DSL_OPENSSL_API extern "C" __declspec(dllimport)
		#define DSL_OPENSSL_API_CLASS __declspec(dllimport)
	#endif
#else
	#define DSL_OPENSSL_API
	#define DSL_OPENSSL_API_CLASS
#endif

class DSL_OPENSSL_API_CLASS DSL_SOCKET_OPENSSL : public DSL_SOCKET {
public:
	SSL * ssl = NULL;
};

class DSL_OPENSSL_API_CLASS DSL_Sockets3_OpenSSL: public DSL_Sockets3_SSL {
	protected:
		virtual DSL_SOCKET * pAllocSocket();
		virtual int pRecv(DSL_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int pPeek(DSL_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int pSend(DSL_SOCKET * sock, const char * buf, uint32 bufsize);
		virtual int pSelect_Read(DSL_SOCKET * sock, timeval * timeo);
		virtual void pCloseSSL(DSL_SOCKET * sock);
	private:
		SSL_CTX * ctx = NULL;
	public:
		DSL_Sockets3_OpenSSL();
		virtual ~DSL_Sockets3_OpenSSL();

		virtual bool EnableSSL(const char * cert_fn, DS3_SSL_METHOD method);
		virtual bool SwitchToSSL_Server(DSL_SOCKET * sock);
		virtual bool SwitchToSSL_Client(DSL_SOCKET * sock);

		virtual X509 * GetSSL_Cert(DSL_SOCKET * sock);
		virtual SSL_CTX * GetSSL_CTX();
};

#endif // __DRIFT_OPENSSL_H__
