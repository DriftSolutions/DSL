//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

 /****************************************************\
| Titus Sockets Class 1.1 | Revision Date: 10/06/2005  |
|      Copyright 2005 Drift Solutions / Indy Sams.     |
|      www.driftsolutions.com / www.titus-dev.com      |
|  File released under the Drift Public License (DPL)  |
 \****************************************************/

#ifndef __DSL_SOCKETS3_H__
#define __DSL_SOCKETS3_H__

#include <drift/Mutex.h>
#include <set>

#define MAX_SOCKETS 1024
#define ADDRLEN 128
#define TS3_MAX_HOSTLEN 40
#define TS3_MAX_SERVLEN 8

class TITUS_SOCKET {
public:
	SOCKET sock = 0;
	uint8 flags = 0;

	char remote_ip[TS3_MAX_HOSTLEN] = { 0 };
	char local_ip[TS3_MAX_HOSTLEN] = { 0 };
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
typedef TITUS_SOCKET T_SOCKET;

struct TITUS_SOCKET_LIST {
	uint32 num;
	T_SOCKET * socks[256];
};

DSL_API void DSL_CC TFD_ZERO(TITUS_SOCKET_LIST * x);
DSL_API void DSL_CC TFD_SET(TITUS_SOCKET_LIST * x, TITUS_SOCKET * sock);
//DSL_API void DSL_CC TFD_UNSET(TITUS_SOCKET_LIST * x, TITUS_SOCKET * sock);
DSL_API bool DSL_CC TFD_ISSET(TITUS_SOCKET_LIST * list, TITUS_SOCKET * sock);

enum TS3_SSL_METHOD {
	TS3_SSL_METHOD_TLS		= 0,	// Attempt highest TLS version, falling back to lower versions to 1.0

	TS3_SSL_METHOD_TLS1_2	= 1,
	TS3_SSL_METHOD_TLS1_1	= 2,
	TS3_SSL_METHOD_TLS1_0	= 3,	// this is the minimum you should use

	TS3_SSL_METHOD_SSL3		= 4,	// SSL v3 Only - DO NOT USE UNLESS YOU HAVE TO!!!
#if !defined(NO_DTLS1)
	TS3_SSL_METHOD_DTLS1	= 5,	// Datagram TLS, consider this untested
#endif
	TS3_SSL_METHOD_DEFAULT	= TS3_SSL_METHOD_TLS,
};

class DSL_API_CLASS Titus_Sockets3_Base {
	protected:
		char bError[512];
		int bErrNo;
		bool silent;
		void pUpdateError(TITUS_SOCKET * sock);
		void pUpdateError(TITUS_SOCKET * sock, int serrno, const char * errstr);

		unsigned int avail_flags = 0;
		unsigned int enabled_flags = 0;

		virtual TITUS_SOCKET * pAllocSocket();
		virtual int pRecv(TITUS_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int pPeek(TITUS_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int pSend(TITUS_SOCKET * sock, const char * buf, uint32 bufsize);
		virtual int pSelect_Read(TITUS_SOCKET * sock, timeval * timeo);

	private:
		Titus_Mutex hMutex;
		typedef std::set<TITUS_SOCKET *> knownSocketList;
		knownSocketList sockets;
		bool pUpdateAddrInfo(TITUS_SOCKET * sock);
		addrinfo * pResolve(TITUS_SOCKET * sock, const char * host, int port);
		void pFreeAddrInfo(addrinfo * ai);

	public:
		Titus_Sockets3_Base();
		virtual ~Titus_Sockets3_Base();

		virtual bool IsSupported(unsigned int flag);//this lets you know if a feature is supported
		virtual bool IsEnabled(unsigned int flag);// this lets you know if a feature is supported and activated (ie. EnableSSL() has been called)
		virtual void Silent(bool bSilent);

		virtual int GetLastError(T_SOCKET * sock=NULL);
		virtual const char * GetLastErrorString(T_SOCKET * sock=NULL);

		//for IPv4: PF_INET
		//for IPv6: PF_INET6
		virtual TITUS_SOCKET * Create(int family = PF_INET, int type = SOCK_STREAM, int proto = IPPROTO_TCP, uint32 flags=0);
		virtual int Close(TITUS_SOCKET * sock);
		virtual bool IsKnownSocket(TITUS_SOCKET * sock);

		virtual int GetFamilyHint(const char * host, int port);
		virtual bool Connect(TITUS_SOCKET * sock, const char * host, int port);
		virtual bool Connect(TITUS_SOCKET * sock, sockaddr * addr, size_t addrlen);
		virtual bool ConnectWithTimeout(TITUS_SOCKET * sock, const char * host, int port, uint32 timeout); // connect with timeout in milliseconds

		virtual int Send(TITUS_SOCKET * sock, const char * data, int datalen = -1, bool doloop = true);
		virtual int Recv(TITUS_SOCKET * sock, char * buf, uint32 bufsize);
		virtual int RecvLine(TITUS_SOCKET * sock, char * buf, int bufsize);
		virtual int Peek(TITUS_SOCKET * sock, char * buf, uint32 bufsize);

		virtual int Select_Read(TITUS_SOCKET * sock, timeval * timeo);
		virtual int Select_Read(TITUS_SOCKET * sock, uint32 millisec);
		virtual int Select_Write(TITUS_SOCKET * sock, timeval * timeo);
		virtual int Select_Write(TITUS_SOCKET * sock, uint32 millisec);

		virtual int Select_Read_List(TITUS_SOCKET_LIST * list, timeval * timeo);
		virtual int Select_Read_List(TITUS_SOCKET_LIST * list, uint32 millisec);
		virtual int Select_List(TITUS_SOCKET_LIST * list_r, TITUS_SOCKET_LIST * list_w, timeval * timeo);
		virtual int Select_List(TITUS_SOCKET_LIST * list_r, TITUS_SOCKET_LIST * list_w, uint32 millisec);

		virtual TITUS_SOCKET * Accept(TITUS_SOCKET * s, sockaddr *addr, socklen_t *addrlen, uint32 flags=0);
		virtual TITUS_SOCKET * Accept(TITUS_SOCKET * s, uint32 flags=0);
		// Bind() is only for IPv4/6
		virtual bool Bind(TITUS_SOCKET * sock, int port);
		virtual bool Listen(TITUS_SOCKET * s, int backlog=5);

		virtual int RecvFrom(TITUS_SOCKET * sock, char * host, uint32 hostSize, int * port, char * buf, uint32 bufsize);
		virtual int PeekFrom(TITUS_SOCKET * sock, char * host, uint32 hostSize, int * port, char * buf, uint32 bufsize);
		virtual int RecvLineFrom(TITUS_SOCKET * sock, char * host, uint32 hostSize, int * port, char * buf, uint32 bufsize);
		virtual int SendTo(TITUS_SOCKET * sock, const char * host, int port, const char * buf, int datalen=-1);
		virtual bool BindToAddr(TITUS_SOCKET * sock, const char * host, int port);

		virtual int SetRecvTimeout(TITUS_SOCKET * sock, uint32 millisec);
		virtual int SetSendTimeout(TITUS_SOCKET * sock, uint32 millisec);

		virtual bool IsNonBlocking(TITUS_SOCKET * sock);
		virtual void SetNonBlocking(TITUS_SOCKET * sock, bool non_blocking=true);
		virtual int SetReuseAddr(TITUS_SOCKET * sock, bool reuse_addr=true);
		virtual int SetLinger(TITUS_SOCKET * sock, bool linger, unsigned short timeo=30);
		virtual int SetNoDelay(TITUS_SOCKET * sock, bool no_delay=true);
		virtual int SetKeepAlive(TITUS_SOCKET * sock, bool ka=true);
		virtual int SetBroadcast(TITUS_SOCKET * sock, bool broadcast=true);
		virtual bool DisableUDPConnReset(TITUS_SOCKET * sock, bool noconnreset);

		virtual std::string GetHostIP(const char * host, int type=SOCK_STREAM, int proto=IPPROTO_TCP);
};

class DSL_API_CLASS Titus_Sockets3_SSL: public Titus_Sockets3_Base {
	friend class Titus_Sockets3_Base;
	protected:
		virtual TITUS_SOCKET * pAllocSocket() = 0;
		virtual int pRecv(TITUS_SOCKET * sock, char * buf, uint32 bufsize) = 0;
		virtual int pPeek(TITUS_SOCKET * sock, char * buf, uint32 bufsize) = 0;
		virtual int pSend(TITUS_SOCKET * sock, const char * buf, uint32 bufsize) = 0;
		virtual void pCloseSSL(TITUS_SOCKET * sock) = 0;
		virtual int pSelect_Read(TITUS_SOCKET * sock, timeval * timeo) { return Titus_Sockets3_Base::pSelect_Read(sock, timeo); }
	public:
		virtual ~Titus_Sockets3_SSL() {}

		virtual bool EnableSSL(const char * cert_fn, TS3_SSL_METHOD method) = 0;
		virtual bool SwitchToSSL_Server(TITUS_SOCKET * sock) = 0;
		virtual bool SwitchToSSL_Client(TITUS_SOCKET * sock) = 0;
};

#if defined(ENABLE_OPENSSL)
#define Titus_Sockets3 Titus_Sockets3_OpenSSL
#elif defined(ENABLE_GNUTLS)
#define Titus_Sockets3 Titus_Sockets3_GnuTLS
#else
#define Titus_Sockets3 Titus_Sockets3_Base
#endif

#define RL3_ERROR			-4
#define RL3_CLOSED			-3
#define RL3_LINETOOLONG 	-2
#define RL3_NOLINE			-1

#define TS3_FLAG_SSL		0x00000001
#define TS3_FLAG_ZIP		0x00000002

#define Titus_Sockets Titus_Sockets3

#endif // __TITUS_SOCKETS3_H__
