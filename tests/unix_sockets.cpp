//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dsl.h>

#define TEST_SOCK_NAME "test.sock"

DSL_Sockets3 * socks = NULL;

DSL_DEFINE_THREAD(ConnectTest) {
	DSL_THREAD_START
	D_SOCKET * sock = socks->Create(AF_UNIX, SOCK_STREAM, 0);
	if (sock != NULL) {
		if (socks->Connect(sock, TEST_SOCK_NAME, 0)) {
			printf("Connected...\n");
			int n = socks->Send(sock, "xxx", 3);
			printf("Send result: %d\n", n);
		} else {
			printf("Error connecting: %s\n", socks->GetLastErrorString(sock));
		}
		socks->Close(sock);
	}
	DSL_THREAD_END
}

int main(int argc, char * argv[]) {
	if (!dsl_init()) {
		printf("dsl_init() failed!\n");
		return 1;
	}

	socks = new DSL_Sockets3();

	D_SOCKET * sock = socks->Create(AF_UNIX, SOCK_STREAM, 0);
	if (sock == NULL) {
		printf("Error connecting: %s\n", socks->GetLastErrorString());
		delete socks;
		return 1;
	}

	if (!socks->BindToAddr(sock, TEST_SOCK_NAME, 0)) {
		printf("Error binding: %s\n", socks->GetLastErrorString(sock));
		socks->Close(sock);
		delete socks;
		return 1;
	}

	if (!socks->Listen(sock)) {
		printf("Error listening: %s\n", socks->GetLastErrorString(sock));
		socks->Close(sock);
		delete socks;
		unlink(TEST_SOCK_NAME);
		return 1;
	}

	DSL_StartThread(ConnectTest, NULL);

	int ret = 1;
	D_SOCKET * s = socks->Accept(sock);
	if (s != NULL) {
		printf("Accepted connection from %s to %s ...\n", s->local_ip, s->remote_ip);
		char buf[32];
		int n = socks->Recv(s, buf, sizeof(buf) - 1);
		if (n > 0) {
			buf[n] = 0;
			printf("Recv: %s\n", buf);
			ret = 0;
		} else if (n == 0) {
			printf("Peer closed while trying to receive: %s\n", socks->GetLastErrorString(s));
		} else {
			printf("Error receiving: %s\n", socks->GetLastErrorString(s));
		}
		socks->Close(s);
	} else {
		printf("Error accepting socket: %s\n", socks->GetLastErrorString(sock));
	}

	socks->Close(sock);

	while (DSL_NumThreads()) {
		safe_sleep(100, true);
	}

	delete socks;
	unlink(TEST_SOCK_NAME);

	dsl_cleanup();
	return ret;
}
