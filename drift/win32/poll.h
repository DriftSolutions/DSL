//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

/*
 * If we are compiling with support for Windows versions before Vista provide our own wrapper functions for these unsupported calls.
 * If the app is running on Vista+ it tries to load the functions dynamically and use the native functions, on XP it:
 * 1) Emulates poll() with select()
 * 2) Uses the old inet_addr/inet_ntoa functions (which means they only support IPv4.)
 */

#ifndef __DSL_POLL_H__
#define __DSL_POLL_H__

#if defined(WIN32)
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0600)

#define POLLRDNORM  0x0100
#define POLLRDBAND  0x0200
#define POLLIN      (POLLRDNORM | POLLRDBAND)

#define POLLWRNORM  0x0010
#define POLLOUT     POLLWRNORM

#define POLLERR     0x0001
#define POLLNVAL    0x0004

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct pollfd {

		SOCKET  fd;
		SHORT   events;
		SHORT   revents;

	} WSAPOLLFD, *PWSAPOLLFD, FAR *LPWSAPOLLFD;

	DSL_API PCSTR DSL_CC dsl_inet_ntop(INT Family, PVOID pAddr, PSTR pStringBuf, size_t StringBufSize);
	DSL_API INT DSL_CC dsl_inet_pton(INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
	DSL_API int DSL_CC dsl_poll(pollfd * fds, int nfds, int timeout);

	#define inet_ntop(w,x,y,z) dsl_inet_ntop(w,x,y,z)
	#define inet_pton(x,y,z) dsl_inet_pton(x,y,z)
	#define poll(x,y,z) dsl_poll(x,y,z)

#ifdef __cplusplus
}
#endif

#endif // !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0600)
#endif // defined(WIN32)
#endif // __DSL_POLL_H__
