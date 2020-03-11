//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dslcore.h>
#include <drift/sockets3.h>
#include <drift/Threading.h>
#include <drift/GenLib.h>

DSL_Mutex  * resMutex()
{
  static DSL_Mutex actualMutex;
  return &actualMutex;
}

/*
struct DSL_SOCKET_IO {
	void (*read)(DSL_SOCKET * sock, char * buf, uint32 bufSize);
	void (*peek)(DSL_SOCKET * sock, char * buf, uint32 bufSize);
	void (*write)(DSL_SOCKET * sock, char * buf, uint32 bufSize);
	void (*close)(DSL_SOCKET * sock);
};
*/

void DSL_CC DFD_ZERO(DSL_SOCKET_LIST * x) { x->num = 0; }
void DSL_CC DFD_SET(DSL_SOCKET_LIST * x, DSL_SOCKET * sock) { x->socks[x->num++] = sock; }
bool DSL_CC DFD_ISSET(DSL_SOCKET_LIST * x, DSL_SOCKET * sock) {
	for (uint32 i=0; i < x->num; i++) {
		if (x->socks[i] == sock) {
			return true;
		}
	}
	return false;
}

void DSL_Sockets3_Base::Silent(bool bSilent) {
	silent = bSilent;
}

DSL_Sockets3_Base::DSL_Sockets3_Base() {
	dsl_init();
#ifdef ENABLE_ZLIB
	avail_flags |= DS3_FLAG_ZIP;
	enabled_flags |= DS3_FLAG_ZIP;
#endif
	memset(bError,0,sizeof(bError));
	bErrNo = 0;
#if defined(DEBUG)
	silent = false;
#else
	silent = true;
#endif
	//hMutex.Release();
}

DSL_Sockets3_Base::~DSL_Sockets3_Base() {
	hMutex.Lock();
	for (knownSocketList::const_iterator i = sockets.begin(); i != sockets.end(); i++) {
		if (!silent) { printf("WARNING: DSL_SOCKET 0x%p (%s:%d) was not closed before DSL_Sockets3 was deleted!\n", *i, (*i)->remote_ip, (*i)->remote_port); }
		//Close(sockets[i]);
	}
	hMutex.Release();
	dsl_cleanup();
}

#if !defined(WIN32)
#define WspiapiGetAddrInfo getaddrinfo
#define WspiapiFreeAddrInfo freeaddrinfo
#endif

addrinfo * DSL_Sockets3_Base::pResolve(DSL_SOCKET * sock, const char * host, int port) {
	char buf[64]={0};
#if defined(itoa)
	itoa(port, buf, 10);
#else
	sprintf(buf, "%d", port);
#endif

	struct addrinfo hints, *ret;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = sock->family;
	hints.ai_socktype = sock->type;
	hints.ai_protocol = sock->proto;
	resMutex()->Lock();
	int error = WspiapiGetAddrInfo(host, buf, &hints, &ret);//getaddrinfo(
	resMutex()->Release();
	if (error) {
		pUpdateError(sock);
		return NULL;
	}

	return ret;
}

void DSL_Sockets3_Base::pFreeAddrInfo(addrinfo * ai) {
	WspiapiFreeAddrInfo(ai);//freeaddrinfo(
}

bool DSL_Sockets3_Base::pUpdateAddrInfo(DSL_SOCKET * sock) {
	sockaddr_storage addr;
	socklen_t addrLen = sizeof(addr);

	sock->remote_ip[0]=0;
	sock->remote_port=0;
	sock->local_ip[0]=0;
	sock->local_port=0;

	bool ret = true;

	char port[DS3_MAX_SERVLEN]={0};
	if (getpeername(sock->sock, (sockaddr *)&addr, &addrLen) == 0) {
		if (getnameinfo((sockaddr *)&addr, addrLen, sock->remote_ip, sizeof(sock->remote_ip), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV) == 0) {
			sock->remote_port = atoi(port);
		} else {
			ret = false;
		}
	} else {
		ret = false;
	}

	addrLen = ADDRLEN;
	if (getsockname(sock->sock, (sockaddr *)&addr, &addrLen) == 0) {
		if (getnameinfo((sockaddr *)&addr, addrLen, sock->local_ip, sizeof(sock->local_ip), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV) == 0) {
			sock->local_port = atoi(port);
		}
	}

	return ret;
}

bool DSL_Sockets3_Base::IsKnownSocket(DSL_SOCKET * sock) {
	AutoMutex(hMutex);
	if (sock != NULL) {
		knownSocketList::const_iterator i = sockets.find(sock);
		if (i != sockets.end()) {
			return true;
		}
	}
	return false;
}

DSL_SOCKET * DSL_Sockets3_Base::pAllocSocket() {
	return new DSL_SOCKET();
}

DSL_SOCKET * DSL_Sockets3_Base::Create(int family, int type, int proto, uint32 flags) {
	DSL_SOCKET * ret = pAllocSocket();

	ret->family = family;
	ret->type = type;
	ret->proto = proto;
	ret->sock = socket(family,type,proto);
#ifdef WIN32
	if (ret->sock == INVALID_SOCKET) {
#else
	if (ret->sock == -1) {
#endif
		pUpdateError(ret);
		delete ret;
		return NULL;
	}

	if (flags & DS3_FLAG_ZIP) {
#ifdef ENABLE_ZLIB
		ret->flags |= DS3_FLAG_ZIP;
#else
		strcpy(bError,"DSL has not been compiled with zlib support");
		bErrNo = 0x54530000;
		this->Close(ret);
		return NULL;
#endif
	}

	hMutex.Lock();
	sockets.insert(ret);
	hMutex.Release();
	return ret;
}

bool DSL_Sockets3_Base::IsEnabled(unsigned int flag) {
	if ((enabled_flags & flag) == flag) {
		return true;
	}
	return false;
}

bool DSL_Sockets3_Base::IsSupported(unsigned int flag) {
	if ((avail_flags & flag) == flag) {
		return true;
	}
	return false;
}

DSL_SOCKET * DSL_Sockets3_Base::Accept(DSL_SOCKET * s, uint32 flags) {
	sockaddr_storage addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t addrlen = sizeof(addr);
	return Accept(s, (sockaddr *)&addr, &addrlen, flags);
}

DSL_SOCKET * DSL_Sockets3_Base::Accept(DSL_SOCKET * s, sockaddr *addr, socklen_t *addrlen, uint32 flags) {
	DSL_SOCKET * ret = pAllocSocket();
	memset(addr, 0, *addrlen);
	ret->sock = accept(s->sock,addr,addrlen);
#ifdef WIN32
	if (ret->sock == INVALID_SOCKET) {
#else
	if (ret->sock == -1) {
#endif
		pUpdateError(ret);
		delete ret;
		return NULL;
	}

	hMutex.Lock();
	sockets.insert(ret);
	hMutex.Release();

	pUpdateAddrInfo(ret);

	if (flags & DS3_FLAG_SSL) {
		DSL_Sockets3_SSL * ssl = dynamic_cast<DSL_Sockets3_SSL *>(this);
		if (ssl) {
			if (IsEnabled(DS3_FLAG_SSL)) {
				if (ssl->SwitchToSSL_Server(ret)) {
					ret->flags |= DS3_FLAG_SSL;
				} else {
					Close(ret);
					return NULL;
				}
			} else {
				strcpy(bError, "SSL has not been enabled!");
				bErrNo = 0x54530000;
				this->Close(ret);
				return NULL;
			}
		} else {
			this->Close(ret);
			strcpy(bError, "DSL has not been compiled with SSL support");
			bErrNo = 0x54530000;
			return NULL;
		}
	}

	if (flags & DS3_FLAG_ZIP) {
#ifdef ENABLE_ZLIB
		ret->flags |= DS3_FLAG_ZIP;
#else
		this->Close(ret);
		strcpy(bError,"DSL has not been compiled with zlib support");
		bErrNo = 0x54530000;
		return NULL;
#endif
	}
	return ret;
}

bool DSL_Sockets3_Base::BindToAddr(DSL_SOCKET * sock, const char * host, int port) {
#if defined(DEBUG)
	char buf[NI_MAXHOST], sport[NI_MAXSERV];
#endif

	if (sock->family == AF_UNIX) {
		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = sock->family;
		if (*host == '\0') {
			*addr.sun_path = '\0';
			strlcpy(addr.sun_path + 1, host + 1, sizeof(addr.sun_path) - 1);
		} else {
			sstrcpy(addr.sun_path, host);
			if (access(host, F_OK) == 0) {
				unlink(host);
			}
		}
		int ret = bind(sock->sock, (struct sockaddr*)&addr, sizeof(addr));
		if (ret == 0) {
#if defined(DEBUG)
			if (getnameinfo((struct sockaddr*)&addr, sizeof(addr), buf, sizeof(buf), sport, sizeof(sport), NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
				printf("DSL_Sockets3_Base::BindToAddr() -> Bound to %s:%s\n", buf, sport);
			}
#endif
			return true;
		} else {
			pUpdateError(sock);
			return false;
		}
	}

	bool bret = false;
	bool tried = false;
	addrinfo * ai = pResolve(sock, host, port);
	if (ai) {
		addrinfo * Scan = ai;
		while (!bret && Scan) {
			if (Scan->ai_family == sock->family || sock->family == AF_UNSPEC) {
				if (port > 0 && Scan->ai_family == AF_INET) {
					((sockaddr_in *)Scan->ai_addr)->sin_port = htons(port);
				}
				if (port > 0 && Scan->ai_family == AF_INET6) {
					((sockaddr_in6 *)Scan->ai_addr)->sin6_port = htons(port);
				}
				tried = true;
				int ret = bind(sock->sock, Scan->ai_addr, Scan->ai_addrlen);
				if (ret == 0) {
#if defined(DEBUG)
					if (getnameinfo(Scan->ai_addr, Scan->ai_addrlen, buf, sizeof(buf), sport, sizeof(sport), NI_NUMERICHOST|NI_NUMERICSERV) == 0) {
						printf("DSL_Sockets3_Base::BindToAddr() -> Bound to %s:%s\n", buf, sport);
					}
#endif
					bret = true;
				} else {
					pUpdateError(sock);
#if defined(DEBUG)
					if (getnameinfo(Scan->ai_addr, Scan->ai_addrlen, buf, sizeof(buf), sport, sizeof(sport), NI_NUMERICHOST|NI_NUMERICSERV) == 0) {
						printf("DSL_Sockets3_Base::BindToAddr() -> Error binding %s:%s\n", buf, sport);
					}
#endif
				}
			}
			Scan = Scan->ai_next;
		}
		pFreeAddrInfo(ai);
	}
	if (!tried) {
		pUpdateError(sock, 999, "Could not resolve any addresses matching address family.");
	}
	return bret;
}

bool DSL_Sockets3_Base::Bind(DSL_SOCKET * sock, int port) {
	if (sock->family == AF_UNIX) {
		pUpdateError(sock, 999, "Bind() does not support Unix Domain Sockets.");
		/* use BindToAddr() with the filename in the addr field. port is ignored for AF_UNIX sockets */
		return false;
	}

	int ret=0;
#if !defined(NO_IPV6)
	if (sock->family == AF_INET6) {
		sockaddr_in6 addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(port);
		ret = bind(sock->sock,(const sockaddr *)&addr, sizeof(sockaddr_in6));
	} else {
#endif
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
		ret = bind(sock->sock,(const sockaddr *)&addr, sizeof(sockaddr_in));
#if !defined(NO_IPV6)
	}
#endif
	pUpdateError(sock);
	return (ret == 0) ? true:false;
}

bool DSL_Sockets3_Base::Listen(DSL_SOCKET * sock, int backlog) {
	int ret = listen(sock->sock,backlog);
	if (ret) {
		pUpdateError(sock);
		return false;
	}
	return true;
}

int DSL_Sockets3_Base::Close(DSL_SOCKET * sock) {
	if (sock == NULL) { return -1; }

	hMutex.Lock();
	knownSocketList::iterator x = sockets.find(sock);
	if (x != sockets.end()) {
		sockets.erase(x);
	}
	hMutex.Release();

	if (sock->flags & DS3_FLAG_SSL) {
		DSL_Sockets3_SSL * ssl = dynamic_cast<DSL_Sockets3_SSL *>(this);
		if (ssl) {
			ssl->pCloseSSL(sock);
		}
	}

#if defined(WIN32)
	int ret = closesocket(sock->sock);
#else
	int ret = close(sock->sock);
#endif
	pUpdateError(sock);
	delete sock;
	return ret;
}

int DSL_Sockets3_Base::GetFamilyHint(const char * host, int port) {
	DSL_SOCKET sock;
	sock.family = PF_UNSPEC;
	addrinfo * ai = pResolve(&sock, host, port);
	int ret = PF_INET;
	if (ai) {
		ret = ai->ai_family;
		pFreeAddrInfo(ai);
	}
	return ret;
}

bool DSL_Sockets3_Base::Connect(DSL_SOCKET * sock, sockaddr * addr, size_t addrlen) {
	if (connect(sock->sock,addr,addrlen) != 0) {
		pUpdateError(sock);
		pUpdateAddrInfo(sock);
		return false;
	}
	pUpdateAddrInfo(sock);
	return true;
}

bool DSL_Sockets3_Base::Connect(DSL_SOCKET * sock, const char * host, int port) {
	if (sock->family == AF_UNIX) {
		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = sock->family;
		if (*host == '\0') {
			*addr.sun_path = '\0';
			strlcpy(addr.sun_path + 1, host + 1, sizeof(addr.sun_path) - 1);
		} else {
			sstrcpy(addr.sun_path, host);
		}
		int ret = connect(sock->sock, (struct sockaddr*)&addr, sizeof(addr));
		if (ret == 0) {
			return true;
		} else {
			pUpdateError(sock);
			return false;
		}
	}

	addrinfo * ai = pResolve(sock, host, port);
	bool ret = false;
	if (ai) {
		addrinfo * Scan = ai;
		while (Scan && !(ret = Connect(sock,Scan->ai_addr,Scan->ai_addrlen)) && !sock->nonblocking) {
			Scan = Scan->ai_next;
		}
		if (!ret) { pUpdateError(sock); }
		pFreeAddrInfo(ai);
		return ret;
	} else {
		//pResolve should have set an error
		return false;
	}
}

bool DSL_Sockets3_Base::ConnectWithTimeout(DSL_SOCKET * sock, const char * host, int port, uint32 timeout) {
	if (sock->family == AF_UNIX) {
		pUpdateError(sock, 999, "ConnectWithTimeout() does not support Unix Domain Sockets.");
		return false;
	}

	addrinfo * ai = pResolve(sock, host, port);
	if (ai == NULL) {
		//pResolve should have set an error
		return false;
	}

	bool nb = sock->nonblocking;
	if (!nb) {
		SetNonBlocking(sock,true);
	}

	bool ret = false;

	addrinfo * Scan = ai;
	timeval timeo;
	int tries=5;
	while (Scan && tries--) {
		Connect(sock, Scan->ai_addr, Scan->ai_addrlen);
#if defined(DEBUG)
		printf("DSL_Sockets3_Base::ConnectWithTimeout(%s:%d): Trying %s ...\n", host, port, sock->remote_ip);
#endif
		memset(&timeo,0,sizeof(timeo));
		timeo.tv_sec = (timeout / 1000);
		timeo.tv_usec = timeout - (timeo.tv_sec * 1000);
		timeo.tv_usec *= 1000;//convert to microseconds
		if (Select_Write(sock, &timeo) > 0) {
			if (pUpdateAddrInfo(sock))  { /* select() sometimes returns writeable even on failure, this should catch those edge cases */
				ret = true;
				break;
			}
		}
		Scan = Scan->ai_next;
	}
	//if (!ret) { pUpdateError(sock); }

	if (!nb) {
		SetNonBlocking(sock,false);
	}
	pFreeAddrInfo(ai);
	return ret;
}

int DSL_Sockets3_Base::Send(DSL_SOCKET * sock, const char * data_in, int datalen, bool doloop) {
	char * data = (char *)data_in;
	if (datalen == -1) { datalen = strlen(data); }

#ifdef ENABLE_ZLIB
	if (sock->flags & DS3_FLAG_ZIP) {
		unsigned long dlen = compressBound(datalen)+9;
		char * data2 = (char *)dsl_malloc(dlen);
		if (compress2((Bytef *)data2+9, &dlen, (Bytef *)data, datalen, 5) == Z_OK && dlen < datalen) {
			data2[0] = 'Z';
			memcpy(data2+1, &dlen, 4);
			memcpy(data2+5, &datalen, 4);
			datalen = dlen+9;
		} else {
			data2[0] = 'U';
			memcpy(data2+1, &datalen, 4);
			memcpy(data2+5, data, datalen);
			datalen = datalen+5;
		}
		data = data2;
	}
#endif

	if (datalen == 0) { return 0; }
	int left = datalen;
	int n = 0;
	do {
		int o = pSend(sock,data+n,left);
		switch(o) {
			case -1:
				pUpdateError(sock);
#ifdef ENABLE_ZLIB
				if (sock->flags & DS3_FLAG_ZIP) {
					dsl_free(data);
				}
#endif
				return -1;
			case 0:
				pUpdateError(sock);
#ifdef ENABLE_ZLIB
				if (sock->flags & DS3_FLAG_ZIP) {
					dsl_free(data);
				}
#endif
				return 0;
			default:
				left -= o;
				n += o;
				break;
		}
	} while (left > 0 && doloop);

#ifdef ENABLE_ZLIB
	if (sock->flags & DS3_FLAG_ZIP) {
		dsl_free(data);
	}
#endif
	return n;
}

int DSL_Sockets3_Base::pSend(DSL_SOCKET * sock, const char * data, uint32 datalen) {
	return send(sock->sock, data, datalen, 0);
}

int DSL_Sockets3_Base::SendTo(DSL_SOCKET * sock, const char * host, int port, const char * data_in, int datalen) {
	char * data = (char *)data_in;
	if (sock->flags & DS3_FLAG_SSL) {
		sprintf(bError, "SendTo doesn't work with SSL");
		bErrNo = 0x54530000;
		return -1;
	}

	if (datalen == -1) { datalen = strlen(data); }

#ifdef ENABLE_ZLIB
	if (sock->flags & DS3_FLAG_ZIP) {
		unsigned long dlen = compressBound(datalen)+9;
		char * data2 = (char *)dsl_malloc(dlen);
		if (compress2((Bytef *)data2+9, &dlen, (Bytef *)data, datalen, 5) == Z_OK && dlen < datalen) {
			data2[0] = 'Z';
			memcpy(data2+1, &dlen, 4);
			memcpy(data2+5, &datalen, 4);
			datalen = dlen+9;
		} else {
			data2[0] = 'U';
			memcpy(data2+1, &datalen, 4);
			memcpy(data2+5, data, datalen);
			datalen = datalen+5;
		}
		data = data2;
	}
#endif

	addrinfo * ai = pResolve(sock, host, port);
	if (ai) {
		int ret = sendto(sock->sock,data,datalen,0,ai->ai_addr,ai->ai_addrlen);
		if (ret <= 0) { pUpdateError(sock); }
		pUpdateAddrInfo(sock);
#ifdef ENABLE_ZLIB
		if (sock->flags & DS3_FLAG_ZIP) {
			dsl_free(data);
		}
#endif
		return ret;
	}
	pUpdateError(sock);
#ifdef ENABLE_ZLIB
	if (sock->flags & DS3_FLAG_ZIP) {
		dsl_free(data);
	}
#endif
	return -1;
}

int DSL_Sockets3_Base::RecvFrom(DSL_SOCKET * sock, char * host, uint32 hostSize, int * port, char * buf, uint32 bufsize) {
	if (sock->flags & DS3_FLAG_SSL) {
		sprintf(bError, "RecvFrom doesn't work with SSL");
		bErrNo = 0x54530000;
		return -1;
	}

	sockaddr * addr = (sockaddr *)dsl_malloc(ADDRLEN);
	memset(addr, 0, ADDRLEN);
#if defined(WIN32)
	int addrLen = ADDRLEN;
#else
	socklen_t addrLen = ADDRLEN;
#endif
	int n = recvfrom(sock->sock,buf,bufsize,0, (sockaddr *)addr, &addrLen);
	if (n <= 0) { pUpdateError(sock); }

	if (addr->sa_family == AF_INET) {
		sockaddr_in * p = (sockaddr_in *)addr;
		inet_ntop(addr->sa_family, &p->sin_addr, sock->remote_ip, sizeof(sock->remote_ip));
		sock->remote_port = ntohs(p->sin_port);
	} else if (addr->sa_family == AF_INET6) {
		sockaddr_in6 * p = (sockaddr_in6 *)addr;
		inet_ntop(addr->sa_family, &p->sin6_addr, sock->remote_ip, sizeof(sock->remote_ip));
		sock->remote_port = ntohs(p->sin6_port);
	} else {
		sprintf(bError, "Unknown address family!");
		bErrNo = 0x54530000;
		return -1;
	}

	//pUpdateAddrInfo(sock);
	if (host) {
		memset(host, 0, hostSize);
		strncpy(host, sock->remote_ip, hostSize-1);
	}
	if (port) {
		*port = sock->remote_port;
	}

	dsl_free(addr);
	return n;
}

int DSL_Sockets3_Base::pRecv(DSL_SOCKET * sock, char * buf, uint32 bufsize) {
	int n = recv(sock->sock,buf,bufsize,0);
	if (n <= 0) { pUpdateError(sock); }
	if (n < 0) { n = -1; }
	return n;
}

int DSL_Sockets3_Base::pPeek(DSL_SOCKET * sock, char * buf, uint32 bufsize) {
	int n = recv(sock->sock,buf,bufsize,MSG_PEEK);
	if (n <= 0) { pUpdateError(sock); }
	if (n < 0) { n = -1; }
	return n;
}

int DSL_Sockets3_Base::Recv(DSL_SOCKET * sock, char * buf, uint32 bufsize) {
#ifdef ENABLE_ZLIB
	int n = 0;
	if (sock->flags & DS3_FLAG_ZIP) {
		if (bufsize < 8) {
			bErrNo = 0x54530021;
			strcpy(bError,"Buffer too small");
			return -1;
		}

		n = pRecv(sock, buf, 1);
		if (n <= 0) {
			return n;
		}

		//printf("ZIP-Type: %c\n", buf[0]);
		if (buf[0] == 'Z') {
			pRecv(sock, buf, 8);
			char *p = buf;
			uint32 * sizec = (uint32 *)p;
			p += 4;
			uint32 * sizeu = (uint32 *)p;
			if (*sizeu > bufsize) {
				bErrNo = 0x54530021;
				strcpy(bError,"Buffer too small");
				return -1;
			} else {
				uint32 left = *sizec;
				uLongf size = *sizeu;
				uint32 ind  = 0;
				char * tmp = (char *)dsl_malloc(left);
				while (left) {
					n = pRecv(sock, tmp+ind, left);
					if (n <= 0) {
						return n;
					}
					ind += n;
					left -= n;
				}
				if (uncompress((Bytef *)buf, (uLongf *)&size, (Bytef *)tmp, ind) == Z_OK) {
					dsl_free(tmp);
					return size;
				} else {
					dsl_free(tmp);
					bErrNo = 0x54530022;
					strcpy(bError,"Error uncompressing data stream");
					return -1;
				}
			}
		} else if (buf[0] == 'U') {
			pRecv(sock, buf, 4);
			uint32 * size = (uint32 *)buf;
			if (*size > bufsize) {
				bErrNo = 0x54530021;
				strcpy(bError,"Buffer too small");
				n = -1;
			} else {
				uint32 left = *size;
				uint32 ind = 0;
				while (left) {
					n = pRecv(sock, buf+ind, left);
					if (n <= 0) {
						return n;
					}
					ind += n;
					left -= n;
				}
				return ind;
			}
		} else {
			bErrNo = 0x54530020;
			strcpy(bError,"ERROR: Stream does not appear to be zipped!");
			return -1;
		}
	}
#endif
	return pRecv(sock,buf,bufsize);
}

int DSL_Sockets3_Base::Peek(DSL_SOCKET * sock, char * buf, uint32 bufsize) {
	return pPeek(sock,buf,bufsize);
}

int DSL_Sockets3_Base::PeekFrom(DSL_SOCKET * sock, char * host, uint32 hostSize, int * port, char * buf, uint32 bufsize) {
	if (sock->flags & DS3_FLAG_SSL) {
		sprintf(bError, "PeekFrom doesn't work with SSL");
		bErrNo = 0x54530000;
		return -1;
	}

	sockaddr * addr = (sockaddr *)dsl_malloc(ADDRLEN);
	memset(addr, 0, ADDRLEN);
#if defined(WIN32)
	int addrLen = ADDRLEN;
#else
	socklen_t addrLen = ADDRLEN;
#endif
	int n = recvfrom(sock->sock,buf,bufsize, MSG_PEEK, (sockaddr *)addr, &addrLen);
	if (n <= 0) { pUpdateError(sock); }

	pUpdateAddrInfo(sock);
	if (host) {
		memset(host, 0, hostSize);
		strncpy(host, sock->remote_ip, hostSize-1);
	}
	if (port) {
		*port = sock->remote_port;
	}

	dsl_free(addr);
	return n;
}

int DSL_Sockets3_Base::RecvLine(DSL_SOCKET * sock, char * buf, int bufsize) {
	int n = Peek(sock,buf,bufsize - 1);
	if (n < 0) { return RL3_ERROR; }
	if (n == 0) { return RL3_CLOSED; }

	buf[n]=0;
	char *p = strchr(buf,'\n');
	if (p == NULL) { p = strchr(buf,'\r'); }
	if (p != NULL) {
		n = (p - buf) + 1;
		memset(buf, 0, bufsize);
		Recv(sock,buf,n);
		while (buf[strlen(buf) - 1] == '\n') { buf[strlen(buf) - 1] = 0; }
		while (buf[strlen(buf) - 1] == '\r') { buf[strlen(buf) - 1] = 0; }
		return strlen(buf);
	} else if (n == (bufsize-1)) {
		return RL3_LINETOOLONG;
	}

	return RL3_NOLINE;
}

int DSL_Sockets3_Base::RecvLineFrom(DSL_SOCKET * sock, char * host, uint32 hostSize, int * port, char * buf, uint32 bufsize) {
	int n = PeekFrom(sock, host, hostSize, port, buf,bufsize - 1);
	if (n <= 0) { pUpdateError(sock); }
	if (n < 0) { return RL3_ERROR; }
	if (n == 0) { return RL3_CLOSED; }

	buf[n]=0;
	char *p = strchr(buf,'\n');
	if (p != NULL) {
		n = (p - buf) + 1;
		memset(buf, 0, bufsize);
		RecvFrom(sock, host, hostSize, port, buf,n);
		while (buf[strlen(buf) - 1] == '\n') { buf[strlen(buf) - 1] = 0; }
		while (buf[strlen(buf) - 1] == '\r') { buf[strlen(buf) - 1] = 0; }
		return strlen(buf);
	}

	return RL3_NOLINE;
}

int DSL_Sockets3_Base::Select_Read(DSL_SOCKET * sock, uint32 millisec) {
	timeval timeo;
	timeo.tv_sec = millisec / 1000;
	millisec -= (timeo.tv_sec * 1000);
	timeo.tv_usec = (millisec * 1000);
	return Select_Read(sock, &timeo);
}

int DSL_Sockets3_Base::pSelect_Read(DSL_SOCKET * sock, timeval * timeo) {
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(sock->sock, &fd);
	int ret = select(sock->sock + 1, &fd, NULL, NULL, timeo);
	if (ret < 0) { pUpdateError(sock); }
	return ret;
}

int DSL_Sockets3_Base::Select_Read(DSL_SOCKET * sock, timeval * timeo) {
	return pSelect_Read(sock, timeo);
}

int DSL_Sockets3_Base::Select_List(DSL_SOCKET_LIST * list_r, DSL_SOCKET_LIST * list_w, timeval * timeo) {
	fd_set fd, fd2;
	FD_ZERO(&fd);
	FD_ZERO(&fd2);
	SOCKET high = 0;
	DSL_SOCKET_LIST tmp, tmp2;
	uint32 i;
	if (list_r) {
		memcpy(&tmp, list_r, sizeof(DSL_SOCKET_LIST));
		for (i = 0; i < list_r->num; i++) {
			if (list_r->socks[i]->sock > high) {
				high = list_r->socks[i]->sock;
			}
			FD_SET(list_r->socks[i]->sock, &fd);
		}
		DFD_ZERO(list_r);
	} else {
		DFD_ZERO(&tmp);
	}
	if (list_w) {
		memcpy(&tmp2, list_w, sizeof(DSL_SOCKET_LIST));
		for (i = 0; i < list_w->num; i++) {
			if (list_w->socks[i]->sock > high) {
				high = list_w->socks[i]->sock;
			}
			FD_SET(list_w->socks[i]->sock, &fd2);
		}
		DFD_ZERO(list_w);
	} else {
		DFD_ZERO(&tmp2);
	}
	int ret = select(high+1,list_r ? &fd : NULL,list_w ? &fd2 : NULL,NULL,timeo);
	if (ret < 0) { pUpdateError(NULL); }
	if (list_r) {
		for (i = 0; i < tmp.num; i++) {
			if (FD_ISSET(tmp.socks[i]->sock, &fd)) {
				DFD_SET(list_r, tmp.socks[i]);
			}
		}
	}
	if (list_w) {
		for (i = 0; i < tmp2.num; i++) {
			if (FD_ISSET(tmp2.socks[i]->sock, &fd2)) {
				DFD_SET(list_w, tmp2.socks[i]);
			}
		}
	}
	return ret;
}

int DSL_Sockets3_Base::Select_List(DSL_SOCKET_LIST * list_r, DSL_SOCKET_LIST * list_w, uint32 millisec) {
	timeval timeo;
	timeo.tv_sec = millisec / 1000;
	millisec -= (timeo.tv_sec * 1000);
	timeo.tv_usec = (millisec * 1000);
	return Select_List(list_r, list_w, &timeo);
}

int DSL_Sockets3_Base::Select_Read_List(DSL_SOCKET_LIST * list, timeval * timeo) {
	fd_set fd;
	FD_ZERO(&fd);
	SOCKET high=0;
	DSL_SOCKET_LIST tmp;
	memcpy(&tmp, list, sizeof(DSL_SOCKET_LIST));
	uint32 i;
	for (i=0; i < list->num; i++) {
		if (list->socks[i]->sock > high) {
			high = list->socks[i]->sock;
		}
		FD_SET(list->socks[i]->sock, &fd);
	}
	DFD_ZERO(list);
	int ret = select(high+1,&fd,NULL,NULL,timeo);
	if (ret < 0) { pUpdateError(NULL); }
	for (i=0; i < tmp.num; i++) {
		if (FD_ISSET(tmp.socks[i]->sock, &fd)) {
			DFD_SET(list, tmp.socks[i]);
		}
	}
	return ret;
}

int DSL_Sockets3_Base::Select_Read_List(DSL_SOCKET_LIST * list, uint32 millisec) {
	timeval timeo;
	timeo.tv_sec = millisec / 1000;
	millisec -= (timeo.tv_sec * 1000);
	timeo.tv_usec = (millisec * 1000);
	return Select_Read_List(list, &timeo);
}

int DSL_Sockets3_Base::Select_Write(DSL_SOCKET * sock, timeval * timeo) {
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(sock->sock, &fd);
	int ret = select(sock->sock+1,NULL,&fd,NULL,timeo);
	if (ret < 0) { pUpdateError(sock); }
	return ret;
}

int DSL_Sockets3_Base::Select_Write(DSL_SOCKET * sock, uint32 millisec) {
	timeval timeo;
	timeo.tv_sec = millisec / 1000;
	millisec -= (timeo.tv_sec * 1000);
	timeo.tv_usec = (millisec * 1000);
	return Select_Write(sock, &timeo);

}

int DSL_Sockets3_Base::SetLinger(DSL_SOCKET * sock, bool linger, unsigned short timeo) {
	struct linger lin = { linger ? 1:0, linger ? timeo:0 };
	int ret = setsockopt(sock->sock, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
	pUpdateError(sock);
	return ret;
}

int DSL_Sockets3_Base::SetReuseAddr(DSL_SOCKET * sock, bool reuse_addr) {
	int ra = reuse_addr ? 1:0;
	int ret = setsockopt(sock->sock, SOL_SOCKET, SO_REUSEADDR, (char *)&ra, sizeof(ra));
	pUpdateError(sock);
	return ret;
}

int DSL_Sockets3_Base::SetNoDelay(DSL_SOCKET * sock, bool no_delay) {
#ifdef TCP_NODELAY
	int nodelay = no_delay ? 1:0;
	int ret = setsockopt(sock->sock, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(int));
	pUpdateError(sock);
	return ret;
#else
	pUpdateError(sock, 999, "Feature not supported on this platform");
	return 0;
#endif
}

int DSL_Sockets3_Base::SetKeepAlive(DSL_SOCKET * sock, bool ka) {
	int keepalive = ka ? 1:0;
	int ret = setsockopt(sock->sock,SOL_SOCKET,SO_KEEPALIVE,(char *)&keepalive,sizeof(int));
	pUpdateError(sock);
	return ret;
}

int DSL_Sockets3_Base::SetBroadcast(DSL_SOCKET * sock, bool broadcast) {
	int tmp = broadcast ? 1:0;
	int ret = setsockopt(sock->sock,SOL_SOCKET,SO_BROADCAST,(char *)&tmp,sizeof(tmp));
	pUpdateError(sock);
	return ret;
}

#if defined(WIN32)
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
#endif
bool DSL_Sockets3_Base::DisableUDPConnReset(DSL_SOCKET * sock, bool noconnreset) {
	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = noconnreset ? false:true;
	return (WSAIoctl(sock->sock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL) == 0) ? true:false;
}
#else
bool DSL_Sockets3_Base::DisableUDPConnReset(DSL_SOCKET * sock, bool noconnreset) {
	return true;
}
#endif


int DSL_Sockets3_Base::SetRecvTimeout(DSL_SOCKET * sock, uint32 millisec) {
#ifdef WIN32
	int ret = setsockopt(sock->sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&millisec,sizeof(millisec));
#else
	timeval tv;
	tv.tv_sec = millisec / 1000;
	tv.tv_usec = millisec - (tv.tv_sec * 1000);
	int ret = setsockopt(sock->sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv));
#endif
	pUpdateError(sock);
	return ret;
}

int DSL_Sockets3_Base::SetSendTimeout(DSL_SOCKET * sock, uint32 millisec) {
#ifdef WIN32
	int ret = setsockopt(sock->sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&millisec,sizeof(millisec));
#else
	timeval tv;
	tv.tv_sec = millisec / 1000;
	tv.tv_usec = millisec - (tv.tv_sec * 1000);
	int ret = setsockopt(sock->sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv));
#endif
	pUpdateError(sock);
	return ret;
}

bool DSL_Sockets3_Base::IsNonBlocking(DSL_SOCKET * sock) {
	return sock->nonblocking;
}

void DSL_Sockets3_Base::SetNonBlocking(DSL_SOCKET * sock, bool non_blocking) {
	sock->nonblocking = non_blocking;
#ifdef _WIN32
	unsigned long blah = non_blocking ? 1:0;
	ioctlsocket(sock->sock,FIONBIO,&blah);
#else
	if (non_blocking){
		fcntl(sock->sock, F_SETFL, fcntl(sock->sock, F_GETFL) | O_NONBLOCK);
	} else {
		fcntl(sock->sock, F_SETFL, fcntl(sock->sock, F_GETFL) & ~O_NONBLOCK);
	}
#endif
	pUpdateError(sock);
}

std::string DSL_Sockets3_Base::GetHostIP(const char * host, int type, int proto) {
	struct addrinfo hints, *ret, *scan;
	std::string str="";

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	//hints.ai_socktype = type;
	//hints.ai_protocol = proto;
	resMutex()->Lock();
	//getaddrinfo(
	int error = WspiapiGetAddrInfo(host, "", &hints, &ret);
	resMutex()->Release();
	if (error) {
		pUpdateError(NULL);
		return str;
	}

	char buf[DS3_MAX_HOSTLEN];
	if (ret) {
		//sockaddr_in * sa = (sockaddr_in *)ret->ai_addr;
		scan = ret;
		while (scan) {
			if (getnameinfo(scan->ai_addr, scan->ai_addrlen, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST|NI_NUMERICSERV) == 0) {
				str = buf;
				break;
			}
			scan = scan->ai_next;
		}
		//uint8 * b = (uint8 *)&sa->sin_addr;
		//char buf[16];
		//sprintf(buf, inet_ntoa(sa->sin_addr));
		//str = inet_ntoa(sa->sin_addr);
		WspiapiFreeAddrInfo(ret);//freeaddrinfo(
	}
	return str;
}

int DSL_Sockets3_Base::GetLastError(D_SOCKET * sock) {
	if (sock) { return sock->last_errno; }
	return bErrNo;
}
const char * DSL_Sockets3_Base::GetLastErrorString(D_SOCKET * sock) {
	if (sock) { return sock->last_error; }
	return bError;
}

void DSL_Sockets3_Base::pUpdateError(DSL_SOCKET * sock, int serrno, const char * errstr) {
	bErrNo = errno;
	sstrcpy(bError, errstr);
	if (sock != NULL) {
		sock->last_errno = bErrNo;
		sstrcpy(sock->last_error, bError);
	}
	if (!silent && bErrNo != 0 && bErrNo != EWOULDBLOCK && bErrNo != ENOTCONN && bErrNo != EINPROGRESS && bErrNo != ENOENT) {
		printf("pUpdateError(): %d -> %s\n",bErrNo,bError);
	}
}

void DSL_Sockets3_Base::pUpdateError(DSL_SOCKET * sock) {
#if defined(WIN32)
	bErrNo = WSAGetLastError();
	if (bErrNo) {
		bError[sizeof(bError)-1] = 0;
		if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, bErrNo, LANG_SYSTEM_DEFAULT, bError, sizeof(bError)-1, NULL) == 0) {
			sstrcpy(bError, "Error getting error message!");
		}
	}
#else
	bErrNo = errno;
	if (bErrNo != 0) {
		char *error = strerror(bErrNo);
		if (error) {
			sstrcpy(bError,error);
		} else {
			sprintf(bError,"Unknown Error (Code: %d)",bErrNo);
		}
	}
#endif
	if (sock != NULL) {
		sock->last_errno = bErrNo;
		sstrcpy(sock->last_error, bError);
	}
	if (bErrNo != 0 && bErrNo != EWOULDBLOCK && bErrNo != ENOTCONN && bErrNo != EINPROGRESS && bErrNo != ENOENT && !silent) {
		printf("pUpdateError(): %d -> %s\n",bErrNo,bError);
	}
}
