//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dslcore.h>
#include <drift/download.h>
#include <drift/base64.h>
#include <drift/GenLib.h>
#include <drift/threading.h>

DSL_Download_NoCurl::DSL_Download_NoCurl(const string& url, DSL_Download_Callback callback, const string& user, const string& pass, void * user_ptr) {
	user_agent = "DSL HTTP Downloader Class (Mozilla)";
	socks = new DSL_Sockets3_Base();
	SetCallback(callback, user_ptr);
	SetURL(url);
	SetUserPass(user, pass);
}

DSL_Download_NoCurl::~DSL_Download_NoCurl() {
	if (socks != NULL) {
		delete socks;
		socks = NULL;
	}
}

void DSL_Download_NoCurl::SetCallback(DSL_Download_Callback pcallback, void * puser_ptr) {
	callback = pcallback;
	u_ptr = puser_ptr;
}

bool DSL_Download_NoCurl::SetURL(const string& purl) {
	if (purl.length() < 8 || strncmp(purl.c_str(), "http://", 7)) {
		this->error = TD_INVALID_PROTOCOL;
		return false;
	}

	host.clear();
	path.clear();
	port = 80;
	error = TD_NO_ERROR;

	char * murl = dsl_strdup(purl.c_str());
	char * begin = murl + 7;

	char * host = begin;
	char *p = strchr(begin, '@');
	if (p != NULL) {
		host = p + 1;
		*p = NULL;
		char * q = strchr(begin, ':');
		if (q != NULL) {
			*q = 0;
			q++;
			SetUserPass(begin, q);
		} else {
			SetUserPass(begin, "");
		}
	}

	p = strchr(host, '/');
	if (p != NULL) {
		path = p;
		*p = 0;
	} else {
		path = "/";
	}

	p = strchr(host, ':');
	if (p != NULL) {
		*p = 0;
		p++;
		port = atoi(p);
	}
	this->host = host;
	if (this->host.empty() || this->path.empty() || port == 0) {
		this->error = TD_INVALID_URL;
		dsl_free(murl);
		return false;
	}

	printf("host: %s, port: %u, path: %s\n", this->host.c_str(), port, this->path.c_str());

	dsl_free(murl);
	return true;
}

bool DSL_Download_NoCurl::GetURL(string& str) {
	if (host.length() && path.length() && port) {
		stringstream url;
		url << "http://";
		if (user.length() || pass.length()) {
			url << user << ":" << pass << "@";
		}
		url << host << ":" << port << path;
		str = std::move(url.str());
		return true;
	}
	return false;
}

void DSL_Download_NoCurl::SetUserPass(const string& puser, const string& ppass) {
	user = puser;
	pass = ppass;
}

void DSL_Download_NoCurl::SetTimeout(uint32 millisec) {
	timeo = millisec;
}

void DSL_Download_NoCurl::FollowRedirects(bool follow) {
	followRedirects = follow;
}

void DSL_Download_NoCurl::SetUserAgent(const string& ua) {
	if (ua.length()) {
		user_agent = ua;
	}
}

bool DSL_Download_NoCurl::Download(DSL_FILE * fWriteTo) {
	if (this->error != TD_NO_ERROR) { return false; }

	D_SOCKET * sock = socks->Create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == NULL) {
		this->error = TD_ERROR_CREATING_SOCKET;
		return false;
	}

	uint32 ctimeo = timeo;
	if (ctimeo == 0) { ctimeo = 60000; }
	if (!socks->ConnectWithTimeout(sock, host.c_str(), port, ctimeo)) {
		this->error = TD_ERROR_CONNECTING;
		socks->Close(sock);
		return false;
	}

	stringstream req;
	req << "GET " << path << " HTTP/1.0\r\n";
	req << "Host: " << host << ":" << port << "\r\n";
	req << "User-Agent: " << user_agent << "\r\n";
	req << "Connection: close\r\n";

	if (user.length() || pass.length()) {
		string auth = mprintf("%s:%s", user.c_str(), pass.c_str());
		size_t len = base64_encode_buffer_size(auth.length());
		char * tmp = (char *)dsl_zmalloc(len + 1);
		base64_encode(auth.c_str(), auth.length(), tmp);
		req << "Authorization: Basic " << tmp << "\r\n";
		dsl_free(tmp);
	}

	req << "\r\n";
//	printf("RAW GET: %s", buf);
	if (timeo) {
		socks->SetRecvTimeout(sock, timeo);
		socks->SetSendTimeout(sock, timeo);
	}
	if (socks->Send(sock, req.str().c_str(), (int)req.str().length()) < (int)req.str().length()) {
		this->error = TD_TIMEOUT;
		socks->Close(sock);
		return false;
	}

	uint64 got = 0, fullsize = 0;

	int n=0,ln=0,tries=0;
	char buf[16384] = { 0 };
	while ((n = socks->RecvLine(sock,buf,16384)) >= RL3_NOLINE) {
		if (n == RL3_NOLINE) {
			if (tries > 300) {
				this->error = TD_INVALID_RESPONSE;
				socks->Close(sock);
				return false;
			} else {
				tries++;
				safe_sleep(100,true);
				continue;
			}
		}
		buf[n]=0;
		while (buf[strlen(buf) - 1] == '\n') { buf[strlen(buf) - 1] = 0; }
		while (buf[strlen(buf) - 1] == '\r') { buf[strlen(buf) - 1] = 0; }
		while (buf[strlen(buf) - 1] == '\n') { buf[strlen(buf) - 1] = 0; }
		if (strlen(buf) == 0) { break; }

		ln++;
		if (ln == 1) {
			if (strstr(buf,"401")) {
				this->error = TD_BAD_USER_PASS;
				socks->Close(sock);
				return false;
			}
			if (strstr(buf,"404")) {
				this->error = TD_FILE_NOT_FOUND;
				socks->Close(sock);
				return false;
			}
			if (!strstr(buf,"200") && !strstr(buf,"302")) {
				this->error = TD_INVALID_RESPONSE;
				socks->Close(sock);
				return false;
			}
		}

		if (!strnicmp(buf,"Content-Length:",strlen("Content-Length:"))) {
			char * p = buf + strlen("Content-Length:");
			if (p[0] == ' ') { p++; }
			fullsize = atoi64(p);
			if (callback != NULL && !callback(0, fullsize, u_ptr)) {
				this->error = TD_CALLBACK_ABORT;
				socks->Close(sock);
				return false;
			}
		}

		if (!strnicmp(buf,"Location:",strlen("Location:"))) {
			char * p = buf + strlen("Location:");
			if (p[0] == ' ') { p++; }

			if (followRedirects) {
				socks->Close(sock);

				string url = mprintf("http://%s:%u%s", host.c_str(), this->port, path.c_str());
				DSL_Download_NoCurl * dl = new DSL_Download_NoCurl(url.c_str(), callback, user, pass, u_ptr);
				dl->SetTimeout(timeo);
				dl->SetUserAgent(user_agent);
				if (dl->GetError() == TD_NO_ERROR) {
					bool ret = dl->Download(fWriteTo);
					this->error = dl->GetError();
					delete dl;
					return ret;
				} else {
					this->error = dl->GetError();
					delete dl;
					return false;
				}
			} else {
				socks->Close(sock);
				this->error = TD_REDIRECT;
				return false;
			}
		}
	}

	while ((n = socks->Recv(sock, buf, 16384)) > 0) {
		//buf[n]=0;
		if (fWriteTo->write(buf, n, fWriteTo) < n) {
			this->error = TD_FILE_WRITE_ERROR;
			socks->Close(sock);
			return false;
		}
		got += n;
		if (callback != NULL && !callback(got, fullsize, u_ptr)) {
			this->error = TD_CALLBACK_ABORT;
			socks->Close(sock);
			return false;
		}
	}

	socks->Close(sock);
	return true;
}

//#endif // !defined(NO_CURL)
