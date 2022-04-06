//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2022 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef ENABLE_LIBEVENT
#include <drift/dslcore.h>
#include <drift/sockets3.h>
#include <drift/libevent.h>
#include <assert.h>
#include <event2/thread.h>

void dsl_libevent_init() {
#ifdef WIN32
	evthread_use_windows_threads();
#else
	evthread_use_pthreads();
#endif
}

void ev_read_cb(evutil_socket_t lsock, short events, void * ptr) {
	DSL_SOCKET_LIBEVENT * s = (DSL_SOCKET_LIBEVENT *)ptr;
	s->read_cb(s, events);
}

void ev_write_cb(evutil_socket_t lsock, short events, void * ptr) {
	DSL_SOCKET_LIBEVENT * s = (DSL_SOCKET_LIBEVENT *)ptr;
	if (s->connecting && s->connect_cb != NULL) {
		s->connecting = false;
		s->connect_cb(s, events);
	} else {
		s->write_cb(s, events);
	}
}

DSL_Sockets_Events::DSL_Sockets_Events(DSL_Sockets3_Base * pSocks) {
	dsl_libevent_init();
	socks = pSocks;
	evbase = event_base_new();
}

DSL_Sockets_Events::~DSL_Sockets_Events() {
	assert(sockets.size() == 0);
	if (evbase != NULL) {
		event_base_free(evbase);
		evbase = NULL;
	}
}

int DSL_Sockets_Events::LoopWithFlags(int flags) {
	return event_base_loop(evbase, flags);
}

int DSL_Sockets_Events::LoopWithTimeout(int timeout) {
	assert(timeout > 0);
	timeval tv;
	tv.tv_sec = (timeout / 1000);
	timeout -= tv.tv_sec * 1000;
	tv.tv_usec = timeout * 1000;
	event_base_loopexit(evbase, &tv);
	return event_base_loop(evbase, 0);
}

void DSL_Sockets_Events::LoopBreak() {
	event_base_loopbreak(evbase);
}

DSL_SOCKET_LIBEVENT * DSL_Sockets_Events::Add(DSL_SOCKET * sock, dsl_sockets_event_callback pread_cb, dsl_sockets_event_callback pwrite_cb, dsl_sockets_event_callback pconnect_cb, void * puser_ptr, bool persist_recv, bool persist_write) {
	assert(sock != NULL);
	if (pread_cb == NULL && pwrite_cb == NULL && pconnect_cb == NULL) {
		assert(0);
		return NULL;
	}
	LockMutex(socks->hMutex);
	DSL_SOCKET_LIBEVENT * s = (DSL_SOCKET_LIBEVENT *)dsl_new(DSL_SOCKET_LIBEVENT);
	memset(s, 0, sizeof(DSL_SOCKET_LIBEVENT));
	s->sock = sock;
	s->read_cb = pread_cb;
	s->write_cb = pwrite_cb;
	s->connect_cb = pconnect_cb;
	s->connecting = (pconnect_cb != NULL);
	s->user_ptr = puser_ptr;

	if (pread_cb != NULL) {
		short flags = EV_READ;
		if (persist_recv) {
			flags |= EV_PERSIST;
		}
		s->evread = event_new(evbase, sock->sock, flags, ev_read_cb, s);
	} else {
		s->evread = NULL;
	}

	if (pread_cb != NULL || pconnect_cb != NULL) {
		short flags = EV_WRITE;
		if (persist_write) {
			flags |= EV_PERSIST;
		}
		s->evwrite = event_new(evbase, sock->sock, flags, ev_write_cb, s);
	} else {
		s->evwrite = NULL;
	}

	sockets.insert(s);
	return s;
}

void DSL_Sockets_Events::Remove(DSL_SOCKET_LIBEVENT * sock, bool close) {
	assert(sock != NULL);
	LockMutex(socks->hMutex);
	auto x = sockets.find(sock);
	if (x != sockets.end()) {
		if (sock->evread != NULL) {
			event_del(sock->evread);
			event_free(sock->evread);
			sock->evread = NULL;
		}
		if (sock->evwrite != NULL) {
			event_del(sock->evwrite);
			event_free(sock->evwrite);
			sock->evwrite = NULL;
		}
		if (close) {
			socks->Close(sock->sock);
		}
		dsl_free(*x);
		sockets.erase(x);
	}
}

void DSL_Sockets_Events::EnableRecv(DSL_SOCKET_LIBEVENT * s, int timeout) {
	assert(s != NULL);
	assert(s->evread != NULL);
	if (timeout > 0) {
		timeval tv;
		tv.tv_sec = (timeout / 1000);
		timeout -= tv.tv_sec * 1000;
		tv.tv_usec = timeout * 1000;
		event_add(s->evread, &tv);
	} else {
		event_add(s->evread, NULL);
	}
}

void DSL_Sockets_Events::EnableWrite(DSL_SOCKET_LIBEVENT * s, int timeout) {
	assert(s != NULL);
	assert(s->evwrite != NULL);
	if (timeout > 0) {
		timeval tv;
		tv.tv_sec = (timeout / 1000);
		timeout -= tv.tv_sec * 1000;
		tv.tv_usec = timeout * 1000;
		event_add(s->evwrite, &tv);
	} else {
		event_add(s->evwrite, NULL);
	}
}

void DSL_Sockets_Events::DisableRecv(DSL_SOCKET_LIBEVENT * s) {
	assert(s != NULL);
	assert(s->evread != NULL);
	event_del(s->evread);
}

void DSL_Sockets_Events::DisableWrite(DSL_SOCKET_LIBEVENT * s) {
	assert(s != NULL);
	assert(s->evwrite != NULL);
	event_del(s->evwrite);
}

DSL_SOCKET_LIBEVENT * DSL_Sockets_Events::AddTimer(dsl_sockets_event_callback cb, bool persist, void * puser_ptr) {
	assert(cb != NULL);
	LockMutex(socks->hMutex);
	DSL_SOCKET_LIBEVENT * s = (DSL_SOCKET_LIBEVENT *)dsl_new(DSL_SOCKET_LIBEVENT);
	memset(s, 0, sizeof(DSL_SOCKET_LIBEVENT));
	s->read_cb = cb;
	s->user_ptr = puser_ptr;

	s->evread = event_new(evbase, -1, persist ? EV_PERSIST : 0, ev_read_cb, s);
	sockets.insert(s);
	return s;
}

// use EnableRecv/DisableRecv to enable/disable timer
void DSL_Sockets_Events::FreeTimer(DSL_SOCKET_LIBEVENT * timer) {
	assert(timer != NULL);
	Remove(timer, false);
}

#endif // ENABLE_LIBEVENT
