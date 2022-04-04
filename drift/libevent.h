//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2022 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DRIFT_LIBEVENT_H__
#define __DRIFT_LIBEVENT_H__

#include <drift/sockets3.h>
#include <set>
#include <event2/event.h>

#if defined(DSL_DLL) && defined(WIN32)
	#if defined(DSL_LIBEVENT_EXPORTS)
		#define DSL_LIBEVENT_API extern "C" __declspec(dllexport)
		#define DSL_LIBEVENT_API_CLASS __declspec(dllexport)
	#else
		#define DSL_LIBEVENT_API extern "C" __declspec(dllimport)
		#define DSL_LIBEVENT_API_CLASS __declspec(dllimport)
	#endif
#else
	#define DSL_LIBEVENT_API
	#define DSL_LIBEVENT_API_CLASS
#endif

struct DSL_SOCKET_LIBEVENT;

typedef void (*dsl_sockets_event_callback) (DSL_SOCKET_LIBEVENT * sock, short flags);

struct DSL_SOCKET_LIBEVENT {
	/* User-accesible Fields */
	DSL_SOCKET * sock;
	void * user_ptr;

	/* Private Fields */
	event * evread;
	event * evwrite;
	bool connecting;
	dsl_sockets_event_callback read_cb;
	dsl_sockets_event_callback write_cb;
	dsl_sockets_event_callback connect_cb;
};

class DSL_LIBEVENT_API_CLASS DSL_Sockets_Events {
	protected:
	private:
		DSL_Sockets3_Base * socks = NULL;
		event_base * evbase = NULL;
		set<DSL_SOCKET_LIBEVENT *> sockets;
	public:
		DSL_Sockets_Events(DSL_Sockets3_Base * pSocks);
		~DSL_Sockets_Events();

		int LoopWithFlags(int flags);
		int LoopWithTimeout(int timeout);
		void LoopBreak();

		DSL_SOCKET_LIBEVENT * Add(DSL_SOCKET * sock, dsl_sockets_event_callback read_cb = NULL, dsl_sockets_event_callback write_cb = NULL, dsl_sockets_event_callback connect_cb = NULL, void * user_ptr = NULL, bool persist_recv = true, bool persist_write = false);
		void Remove(DSL_SOCKET_LIBEVENT * s, bool close = false); // Removes the socket from libevents leaving it otherwise untouched, optionally closes the socket in the parent DSL_Sockets

		void EnableRecv(DSL_SOCKET_LIBEVENT * s, int timeout = 0);
		void EnableWrite(DSL_SOCKET_LIBEVENT * s, int timeout = 0);
		void DisableRecv(DSL_SOCKET_LIBEVENT * s);
		void DisableWrite(DSL_SOCKET_LIBEVENT * s);

};

#endif // __DRIFT_LIBEVENT_H__
