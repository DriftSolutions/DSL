//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_SOCKETS3_H__
#define __DSL_SOCKETS3_H__

#include <drift/Mutex.h>
#include <set>

#define MAX_SOCKETS 1024
#define ADDRLEN 128
#define DS3_MAX_HOSTLEN 40
#define DS3_MAX_SERVLEN 8

class DSL_SOCKET {
public:
	SOCKET sock = 0;
	uint8 flags = 0;

	char remote_ip[DS3_MAX_HOSTLEN] = { 0 };
	char local_ip[DS3_MAX_HOSTLEN] = { 0 };
	int remote_port = 0;
	int local_port = 0;
	int family = 0;
	int type = 0;
	int proto = 0;;
	bool nonblocking = false;

	int last_errno = 0;
	char last_error[128] = { 0 };

	//this is a user pointer, you can do whatever you want with it
	void * userPtr = NULL;
};
typedef DSL_SOCKET D_SOCKET;

struct DSL_SOCKET_LIST {
	uint32 num;
	D_SOCKET * socks[256];
};

DSL_API void DSL_CC DFD_ZERO(DSL_SOCKET_LIST * x);
DSL_API void DSL_CC DFD_SET(DSL_SOCKET_LIST * x, DSL_SOCKET * sock);
//DSL_API void DSL_CC TFD_UNSET(DSL_SOCKET_LIST * x, DSL_SOCKET * sock);
DSL_API bool DSL_CC DFD_ISSET(DSL_SOCKET_LIST * list, DSL_SOCKET * sock);

enum DS3_SSL_METHOD {
	DS3_SSL_METHOD_TLS		= 0,	// Attempt highest TLS version, falling back to lower versions to 1.0

	DS3_SSL_METHOD_TLS1_2	= 1,
	DS3_SSL_METHOD_TLS1_1	= 2,
	DS3_SSL_METHOD_TLS1_0	= 3,	// this is the minimum you should use

	DS3_SSL_METHOD_SSL3		= 4,	// SSL v3 Only - DO NOT USE UNLESS YOU HAVE TO!!!
#if !defined(NO_DTLS1)
	DS3_SSL_METHOD_DTLS1	= 5,	// Datagram TLS, consider this untested
#endif
	DS3_SSL_METHOD_DEFAULT	= DS3_SSL_METHOD_TLS,
};

class DSL_API_CLASS DSL_Sockets3_Base {
	protected:
		char bError[512];
		int bErrNo;
		bool silent;
		void pUpdateError(DSL_SOCKET * sock);
		void pUpdateError(DSL_SOCKET * sock, int serrno, const char * errstr);

		unsigned int avail_flags = 0;
		unsigned int enabled_flags = 0;

		virtual DSL_SOCKET * pAllocSocket();
		virtual int pRecv(DSL_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int pPeek(DSL_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int pSend(DSL_SOCKET * sock, const char * buf, uint32 bufsize);
		virtual int pSelect_Read(DSL_SOCKET * sock, timeval * timeo);

	private:
		DSL_Mutex hMutex;
		typedef std::set<DSL_SOCKET *> knownSocketList;
		knownSocketList sockets;
		bool pUpdateAddrInfo(DSL_SOCKET * sock);
		addrinfo * pResolve(DSL_SOCKET * sock, const char * host, int port);
		void pFreeAddrInfo(addrinfo * ai);

	public:
		DSL_Sockets3_Base();
		virtual ~DSL_Sockets3_Base();

		virtual bool IsSupported(unsigned int flag);//this lets you know if a feature is supported
		virtual bool IsEnabled(unsigned int flag);// this lets you know if a feature is supported and activated (ie. EnableSSL() has been called)
		virtual void Silent(bool bSilent);

		virtual int GetLastError(D_SOCKET * sock=NULL);
		virtual const char * GetLastErrorString(D_SOCKET * sock=NULL);

		//for IPv4: PF_INET
		//for IPv6: PF_INET6
		virtual DSL_SOCKET * Create(int family = PF_INET, int type = SOCK_STREAM, int proto = IPPROTO_TCP, uint32 flags=0);
		virtual int Close(DSL_SOCKET * sock);
		virtual bool IsKnownSocket(DSL_SOCKET * sock);

		virtual int GetFamilyHint(const char * host, int port);
		virtual bool Connect(DSL_SOCKET * sock, const char * host, int port);
		virtual bool Connect(DSL_SOCKET * sock, sockaddr * addr, size_t addrlen);
		virtual bool ConnectWithTimeout(DSL_SOCKET * sock, const char * host, int port, uint32 timeout); // connect with timeout in milliseconds

		virtual int Send(DSL_SOCKET * sock, const char * data, int datalen = -1, bool doloop = true);
		virtual int Recv(DSL_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int RecvLine(DSL_SOCKET * sock, char * buf, int bufsize);
		virtual int Peek(DSL_SOCKET * sock, char * buf, uint32 bufsize);

		virtual int Select_Read(DSL_SOCKET * sock, timeval * timeo);
		virtual int Select_Read(DSL_SOCKET * sock, uint32 millisec);
		virtual int Select_Write(DSL_SOCKET * sock, timeval * timeo);
		virtual int Select_Write(DSL_SOCKET * sock, uint32 millisec);

		virtual int Select_Read_List(DSL_SOCKET_LIST * list, timeval * timeo);
		virtual int Select_Read_List(DSL_SOCKET_LIST * list, uint32 millisec);
		virtual int Select_List(DSL_SOCKET_LIST * list_r, DSL_SOCKET_LIST * list_w, timeval * timeo);
		virtual int Select_List(DSL_SOCKET_LIST * list_r, DSL_SOCKET_LIST * list_w, uint32 millisec);

		virtual DSL_SOCKET * Accept(DSL_SOCKET * s, sockaddr *addr, socklen_t *addrlen, uint32 flags=0);
		virtual DSL_SOCKET * Accept(DSL_SOCKET * s, uint32 flags=0);
		// Bind() is only for IPv4/6
		virtual bool Bind(DSL_SOCKET * sock, int port);
		virtual bool Listen(DSL_SOCKET * s, int backlog=5);

		virtual int RecvFrom(DSL_SOCKET * sock, char * host, uint32 hostSize, int * port, char * buf, uint32 bufsize);
		virtual int PeekFrom(DSL_SOCKET * sock, char * host, uint32 hostSize, int * port, char * buf, uint32 bufsize);
		virtual int RecvLineFrom(DSL_SOCKET * sock, char * host, uint32 hostSize, int * port, char * buf, uint32 bufsize);
		virtual int SendTo(DSL_SOCKET * sock, const char * host, int port, const char * buf, int datalen=-1);
		virtual bool BindToAddr(DSL_SOCKET * sock, const char * host, int port);

		virtual int SetRecvTimeout(DSL_SOCKET * sock, uint32 millisec);
		virtual int SetSendTimeout(DSL_SOCKET * sock, uint32 millisec);

		virtual bool IsNonBlocking(DSL_SOCKET * sock);
		virtual void SetNonBlocking(DSL_SOCKET * sock, bool non_blocking=true);
		virtual int SetReuseAddr(DSL_SOCKET * sock, bool reuse_addr=true);
		virtual int SetLinger(DSL_SOCKET * sock, bool linger, unsigned short timeo=30);
		virtual int SetNoDelay(DSL_SOCKET * sock, bool no_delay=true);
		virtual int SetKeepAlive(DSL_SOCKET * sock, bool ka=true);
		virtual int SetBroadcast(DSL_SOCKET * sock, bool broadcast=true);
		virtual bool DisableUDPConnReset(DSL_SOCKET * sock, bool noconnreset);

		virtual std::string GetHostIP(const char * host, int type=SOCK_STREAM, int proto=IPPROTO_TCP);
};

class DSL_API_CLASS DSL_Sockets3_SSL: public DSL_Sockets3_Base {
	friend class DSL_Sockets3_Base;
	protected:
		virtual DSL_SOCKET * pAllocSocket() = 0;
		virtual int pRecv(DSL_SOCKET * sock, char * buf, uint32 bufsize) = 0;
		virtual int pPeek(DSL_SOCKET * sock, char * buf, uint32 bufsize) = 0;
		virtual int pSend(DSL_SOCKET * sock, const char * buf, uint32 bufsize) = 0;
		virtual void pCloseSSL(DSL_SOCKET * sock) = 0;
		virtual int pSelect_Read(DSL_SOCKET * sock, timeval * timeo) { return DSL_Sockets3_Base::pSelect_Read(sock, timeo); }
	public:
		virtual ~DSL_Sockets3_SSL() {}

		virtual bool EnableSSL(const char * cert_fn, DS3_SSL_METHOD method) = 0;
		virtual bool SwitchToSSL_Server(DSL_SOCKET * sock) = 0;
		virtual bool SwitchToSSL_Client(DSL_SOCKET * sock) = 0;
};

#if defined(ENABLE_OPENSSL)
#define DSL_Sockets3 DSL_Sockets3_OpenSSL
#elif defined(ENABLE_GNUTLS)
#define DSL_Sockets3 DSL_Sockets3_GnuTLS
#else
#define DSL_Sockets3 DSL_Sockets3_Base
#endif

#define RL3_ERROR			-4
#define RL3_CLOSED			-3
#define RL3_LINETOOLONG 	-2
#define RL3_NOLINE			-1

#define DS3_FLAG_SSL		0x00000001
#define DS3_FLAG_ZIP		0x00000002

#define DSL_Sockets DSL_Sockets3

#endif // __DSL_SOCKETS3_H__
