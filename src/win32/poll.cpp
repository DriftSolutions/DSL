//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#if defined(_WIN32)
#include <drift/dslcore.h>
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0600)
#include <drift/Mutex.h>
#include <drift/DynamicLinking.h>
#include <drift/GenLib.h>
#include <drift/win32/poll.h>
#include <VersionHelpers.h>

typedef int (WSAAPI *WSAPollType)(_Inout_ LPWSAPOLLFD fdArray, _In_ ULONG fds, _In_ INT timeout);
typedef INT(WSAAPI * inet_pton_type)(_In_ INT Family, _In_ PCSTR pszAddrString, _Out_writes_bytes_(sizeof(IN6_ADDR)) PVOID pAddrBuf);
typedef PCSTR(WSAAPI * inet_ntop_type)(_In_ INT Family, _In_ PVOID pAddr, _Out_writes_(StringBufSize) PSTR pStringBuf, _In_ size_t StringBufSize);

WSAPollType WSAPoll = NULL;
inet_pton_type real_inet_pton = NULL;
inet_ntop_type real_inet_ntop = NULL;
bool dsl_socket_shims_has_init = false;

DSL_Mutex * getSocketShimsMutex() {
	static DSL_Mutex SocketShimsMutex;
	return &SocketShimsMutex;
}

void dsl_socket_shims_init() {
	if (!dsl_socket_shims_has_init) {
		/* avoid a lock if we don't have to */
		AutoMutexPtr(getSocketShimsMutex());
		if (!dsl_socket_shims_has_init) { /* in case another thread has done it already */
			if (IsWindowsVistaOrGreater()) {
				DL_HANDLE h = DL_Open("ws2_32.dll");
				if (h != NULL) {
					WSAPoll = (WSAPollType)DL_GetAddress(h, "WSAPoll");
					real_inet_pton = (inet_pton_type)DL_GetAddress(h, "inet_pton");
					real_inet_ntop = (inet_ntop_type)DL_GetAddress(h, "inet_ntop");
					//printf("dsl_socket_shims_init(): %p / %p / %p\n", WSAPoll, real_inet_pton, real_inet_ntop);
				}
			}
			dsl_socket_shims_has_init = true;
		}
	}
}

PCSTR DSL_CC dsl_inet_ntop(INT Family, PVOID pAddr, PSTR pStringBuf, size_t StringBufSize) {
	dsl_socket_shims_init();
	if (real_inet_ntop != NULL) {
		return real_inet_ntop(Family, pAddr, pStringBuf, StringBufSize);
	}

	if (Family != AF_INET) {
		return NULL;
	}

	struct in_addr * tmp = (struct in_addr *)pAddr;
	char * ret = inet_ntoa(*tmp);
	if (ret == NULL) {
		return NULL;
	}

	strlcpy(pStringBuf, ret, StringBufSize);
	return pStringBuf;
}

INT DSL_CC dsl_inet_pton(INT Family, PCSTR pszAddrString, PVOID pAddrBuf) {
	dsl_socket_shims_init();
	if (real_inet_pton != NULL) {
		return real_inet_pton(Family, pszAddrString, pAddrBuf);
	}

	if (Family != AF_INET) {
		return 0;
	}

	struct in_addr * x = (struct in_addr *) pAddrBuf;
	x->S_un.S_addr = inet_addr(pszAddrString);
	if (x->S_un.S_addr == INADDR_NONE || x->S_un.S_addr == INADDR_ANY) {
		x->S_un.S_addr = 0;
		return 0;
	}

	return 1;
}

int DSL_CC dsl_poll(pollfd * fds, int nfds, int timeout) {
	dsl_socket_shims_init();
	if (WSAPoll != NULL) {
		return WSAPoll(fds, nfds, timeout);
	}

	fd_set rd,wr,ex;
	FD_ZERO(&rd);
	FD_ZERO(&wr);
	FD_ZERO(&ex);
	for (int i=0; i < nfds; i++) {
		fds[i].revents = 0;
		int t;
		int tlen = sizeof(t);
		if (getsockopt(fds[i].fd, SOL_SOCKET, SO_TYPE, (char *)&t, &tlen) == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) {
			//not a socket
			fds[i].revents |= POLLNVAL;
		} else {
			if (fds[i].events & POLLRDNORM) {
				FD_SET(fds[i].fd, &rd);
			}
			if (fds[i].events & POLLWRNORM) {
				FD_SET(fds[i].fd, &wr);
			}
			FD_SET(fds[i].fd, &ex);
		}
	}

	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if (timeout > 0) {
		tv.tv_sec = (timeout / 1000);
		timeout -= (tv.tv_sec * 1000);
		tv.tv_usec = timeout * 1000;
	}
	int ret = 0;
	if (select(0, &rd, &wr, &ex, (timeout >= 0) ? &tv : NULL) > 0) {
		for (int i=0; i < nfds; i++) {
			if (FD_ISSET(fds[i].fd, &rd) && (fds[i].events & POLLRDNORM)) {
				fds[i].revents |= POLLRDNORM;
			}
			if (FD_ISSET(fds[i].fd, &wr) && (fds[i].events & POLLWRNORM)) {
				fds[i].revents |= POLLWRNORM;
			}
			if (FD_ISSET(fds[i].fd, &ex)) {
				fds[i].revents |= POLLERR;
			}
			if (fds[i].revents) { ret++; }
		}
	}
	return ret;
}

#endif // !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0600)
#endif // defined(WIN32)
