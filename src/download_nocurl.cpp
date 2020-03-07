//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#include <drift/dslcore.h>
#include <drift/download.h>
#include <drift/base64.h>
#include <drift/GenLib.h>
#include <drift/Threading.h>

void DSL_Download_NoCurl::privZero() {
	this->host[0]=0;
	this->path[0]=0;
	this->user[0]=0;
	this->pass[0]=0;
	this->port=0;
	this->timeo=0;
	this->followRedirects = false;
	this->errorOnRedirects = true;
	this->callback = NULL;
	this->error = TD_NO_ERROR;
	this->socks = new DSL_Sockets3_Base();
}

DSL_Download_NoCurl::DSL_Download_NoCurl(const char * url, DSL_Download_Callback callback, const char * user, const char * pass, void * user_ptr) {
	this->privZero();
	this->callback = callback;
	this->u_ptr = user_ptr;
	this->error = TD_NO_ERROR;
	strcpy(this->user_agent, "DSL HTTP Downloader Class (Mozilla)");
	if (user) { sstrcpy(this->user, user); }
	if (pass) { sstrcpy(this->pass, pass); }

	char * murl = dsl_strdup(url);
	char *p = strchr(murl, '/')+2;

	if (p != (char *)2) {
		p[-1]=0;
		//strlwr(murl);
		if (!stricmp(murl, "http:/")) {
			this->port = 80;
			this->mode = TD_HTTP;
			char * q = strchr(p, '/');
			if (q) {
				sstrcpy(this->path, q);
				q[0]=0;
				if (strchr(p, ':')) {
					char * po = strchr(p, ':');
					po[0]=0;
					po++;
					this->port = atoi(po);
					sstrcpy(this->host, p);
				} else {
					sstrcpy(this->host, p);
				}
			} else {
				this->error = TD_INVALID_URL;
			}
		} else {
			this->error = TD_INVALID_URL;
		}
	} else {
		this->error = TD_INVALID_PROTOCOL;
	}

//	printf("host: %s, port: %d, path: %s\n", host, port, path);

	dsl_free(murl);
}

DSL_Download_NoCurl::DSL_Download_NoCurl(DSL_Download_Type type, const char * host, int port, const char * path, DSL_Download_Callback callback, const char * user, const char * pass, void * user_ptr) {
	this->privZero();
	this->mode = type;
	if (host) { sstrcpy(this->host,host); }
	this->port = port;
	if (path) { sstrcpy(this->path,path); }
	sstrcpy(this->user_agent,"DSL HTTP Downloader Class (Mozilla)");

	this->callback = callback;
	this->u_ptr = user_ptr;
	if (user) {
		sstrcpy(this->user, user);
	} else if (type == TD_FTP) {
		sstrcpy(this->user, "anonymous");
	}
	if (pass) {
		sstrcpy(this->pass, pass);
	} else if (type == TD_FTP) {
		sstrcpy(this->pass, "none@nobody.com");
	}
	this->error = TD_NO_ERROR;
	if (type != TD_HTTP && type != TD_FTP) {
		this->error = TD_INVALID_PROTOCOL;
	}
}

DSL_Download_NoCurl::~DSL_Download_NoCurl() {
	delete this->socks;
}

//DSL_Download_Errors DSL_Download::GetError() { return this->error; }

void DSL_Download_NoCurl::SetTimeout(unsigned long millisec) {
	this->timeo = millisec;
}

void DSL_Download_NoCurl::FollowRedirects(bool follow) {
	this->followRedirects = follow;
}

void DSL_Download_NoCurl::ErrorOnRedirects(bool error) {
	this->errorOnRedirects = error;
}

void DSL_Download_NoCurl::SetUserAgent(const char * ua) {
	strcpy(user_agent, ua);
}

/*
char DSL_Download_Error_Strings[11][256] = {
	"No Error",

	"File Access Error",
	"Error Connecting to Host",
	"Invalid Response",
	"Error Creating Socket",

	"Error writing file",
	"Invalid URL",
	"Callback Abort",
	"Invalid Protocol",
	"File Not Found",
	"Server tried to redirect, but FollowRedirects is Off"
};

const char * DSL_Download::GetErrorString() {
	return DSL_Download_Error_Strings[this->error];
}
*/

bool DSL_Download_NoCurl::Download(const char * SaveAs) {
	if (this->error != TD_NO_ERROR) { return false; }

	DSL_FILE * fp = RW_OpenFile(SaveAs, "wb");
	if (fp == NULL) {
		this->error = TD_FILE_ACCESS;
		return false;
	}
	bool ret = this->Download(fp);
	fp->close(fp);
	return ret;
}

bool DSL_Download_NoCurl::Download(FILE * fWriteTo) {
	if (this->error != TD_NO_ERROR) { return false; }

	DSL_FILE * fp = RW_ConvertFile(fWriteTo, false);
	bool ret = this->Download(fp);
	fp->close(fp);
	return ret;
}

bool DSL_Download_NoCurl::Download(DSL_FILE * fWriteTo) {
	if (this->error != TD_NO_ERROR) { return false; }

	if (this->mode == TD_HTTP) {
		return this->privDownloadHTTP(fWriteTo);
	}

	if (this->mode == TD_FTP) {
		return this->privDownloadFTP(fWriteTo);
	}

	this->error = TD_INVALID_PROTOCOL;
	return false;
}

bool DSL_Download_NoCurl::privDownloadHTTP(DSL_FILE * fWriteTo) {
	char * buf = (char *)dsl_malloc(16384);
	uint64 got=0,fullsize=0;
	D_SOCKET * sock = this->socks->Create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == NULL) {
		dsl_free(buf);
		this->error = TD_ERROR_CREATING_SOCKET;
		printf("Error creating socket! Error %d: %s\n", socks->GetLastError(), socks->GetLastErrorString());
		return false;
	}
	int ctimeo = timeo;
	if (!ctimeo) { ctimeo = 60000; }
	if (!this->socks->ConnectWithTimeout(sock,this->host, this->port, ctimeo)) {
		dsl_free(buf);
		this->error = TD_ERROR_CONNECTING;
		this->socks->Close(sock);
		return false;
	}

	//sprintf(buf,"GET %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: %s\r\nConnection: close\r\n",this->path,this->host,this->port,this->user_agent);
	snprintf(buf, 16384, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\n", this->path, this->host, this->user_agent);

	if (strlen(this->user) || strlen(this->pass)) {
		char buf2[1024],buf3[1024];
		memset(buf2,0,sizeof(buf2));
		memset(buf3,0,sizeof(buf3));
		snprintf(buf2,sizeof(buf2),"%s:%s",this->user,this->pass);
		base64_encode(buf2,strlen(buf2),buf3);
		snprintf(buf2, sizeof(buf2), "Authorization: Basic %s\r\n",buf3);
		strlcat(buf, buf2, 16384);
	}

	strlcat(buf, "\r\n", 16384);
//	printf("RAW GET: %s", buf);
	if (this->timeo) {
		this->socks->SetRecvTimeout(sock, timeo);
	}
	this->socks->Send(sock,buf);

	int n=0,ln=0,tries=0;
	memset(buf,0,16384);
	while ((n = this->socks->RecvLine(sock,buf,16384)) >= RL3_NOLINE) {
		if (n == RL3_NOLINE) {
			if (tries > 300) {
				this->error = TD_INVALID_RESPONSE;
				this->socks->Close(sock);
				dsl_free(buf);
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
				this->socks->Close(sock);
				dsl_free(buf);
				return false;
			}
			if (strstr(buf,"404")) {
				this->error = TD_FILE_NOT_FOUND;
				this->socks->Close(sock);
				dsl_free(buf);
				return false;
			}
			if (!strstr(buf,"200") && !strstr(buf,"302")) {
				this->error = TD_INVALID_RESPONSE;
				this->socks->Close(sock);
				dsl_free(buf);
				return false;
			}
		}

		if (!strnicmp(buf,"Content-Length:",strlen("Content-Length:"))) {
			char * p = buf + strlen("Content-Length:");
			if (p[0] == ' ') { p++; }
			fullsize = atoi64(p);
			if (this->callback != NULL && !this->callback(0,fullsize,u_ptr)) {
				this->error = TD_CALLBACK_ABORT;
				this->socks->Close(sock);
				dsl_free(buf);
				return false;
			}
		}

		if (!strnicmp(buf,"Location:",strlen("Location:"))) {
			char * p = buf + strlen("Location:");
			if (p[0] == ' ') { p++; }

			if (this->followRedirects) {
				this->socks->Close(sock);
				if (p[0] == '/') {
					char * tmp = dsl_strdup(p);
					snprintf(p, 16384 - (p - buf + 1), "http://%s:%d%s", this->host, this->port, tmp);
					dsl_free(tmp);
				}

				DSL_Download_NoCurl * dl = new DSL_Download_NoCurl(p,callback,user,pass);
				if (dl->GetError() == TD_NO_ERROR) {
					bool ret = dl->Download(fWriteTo);
					this->error = dl->GetError();
					delete dl;
					dsl_free(buf);
					return ret;
				} else {
					this->error = dl->GetError();
					delete dl;
					dsl_free(buf);
					return false;
				}
			} else if (this->errorOnRedirects) {
				this->socks->Close(sock);
				this->error = TD_REDIRECT;
				dsl_free(buf);
				return false;
			}
		}
	}

	while ((n = this->socks->Recv(sock,buf,16384)) > 0) {
		//buf[n]=0;
		if (fWriteTo->write(buf,n,fWriteTo) < n) {
			this->error = TD_FILE_WRITE_ERROR;
			this->socks->Close(sock);
			dsl_free(buf);
			return false;
		}
		got += n;
		if (this->callback != NULL && !this->callback(got,fullsize,u_ptr)) {
			this->error = TD_CALLBACK_ABORT;
			this->socks->Close(sock);
			dsl_free(buf);
			return false;
		}
	}

	dsl_free(buf);
	this->socks->Close(sock);
	return true;
}

bool DSL_Download_NoCurl::privDownloadFTP(DSL_FILE * fWriteTo) {
	char * buf = new char[16384];
	long got=0,fullsize=0;
	D_SOCKET * sock = this->socks->Create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == NULL) {
		this->error = TD_ERROR_CREATING_SOCKET;
		return false;
	}
	if (!this->socks->Connect(sock,this->host,this->port)) {
		this->error = TD_ERROR_CONNECTING;
		this->socks->Close(sock);
		return false;
	}

	D_SOCKET * dSock=NULL;

	int n=0,ln=0;
	memset(buf,0,16384);
	while ((n = this->socks->RecvLine(sock,buf,16384)) >= 0) {
		if (n == 0) { continue; }
		buf[n]=0;
		while (buf[strlen(buf) - 1] == '\n') { buf[strlen(buf) - 1] = 0; }
		while (buf[strlen(buf) - 1] == '\r') { buf[strlen(buf) - 1] = 0; }
		while (buf[strlen(buf) - 1] == '\n') { buf[strlen(buf) - 1] = 0; }
		if (strlen(buf) == 0) { break; }

		char *code = buf;
		char *data = strchr(code,' ');
		if (data) {
			data[0]=0;
			data++;
		} else {
			data = code;
		}

		//printf("code: %s, data: %s\n",code,data);
		if (ln == 0) {
			switch(atol(code)) {
				case 220:
					sprintf(buf,"USER %s\r\n",this->user);
					this->socks->Send(sock,buf);
					ln++;
					break;
				default:
					this->error = TD_INVALID_RESPONSE;
					this->socks->Close(sock);
					return false;
					break;
			}
			continue;
		}

		if (ln == 1) { // after USER
			switch(atol(code)) {
				case 331: // username accepted
					sprintf(buf,"PASS %s\r\n",this->pass);
					this->socks->Send(sock,buf);
					ln++;
					break;
				default:
					this->error = TD_INVALID_RESPONSE;
					this->socks->Close(sock);
					return false;
					break;
			}
			continue;
		}

		if (ln == 2) { // after PASS
			switch(atol(code)) {
				case 230: // Access granted, logged in
					sprintf(buf,"TYPE I\r\n");
					this->socks->Send(sock,buf);
					ln++;
					break;
				default:
					this->error = TD_BAD_USER_PASS;
					this->socks->Close(sock);
					return false;
					break;
			}
			continue;
		}

		if (ln == 3) { // after TYPE
			switch(atol(code)) {
				case 200: // Access granted, logged in
					sprintf(buf,"PASV\r\n");
					this->socks->Send(sock,buf);
					ln++;
					break;
				default:
					this->error = TD_INVALID_RESPONSE;
					this->socks->Close(sock);
					return false;
					break;
			}
			continue;
		}

		if (ln == 4) { // after PASV
			switch(atol(code)) {
				case 227:{ // PASV mode accepted
						char *p = strchr(data,'(');
						if (p) {
							p++;
							strchr(p,')')[0]=0;

							int i=0;
							unsigned char ip[4];
							int mport=0;

							p = strtok(p,",");
							while (p) {
								if (i < 4) {
									ip[i] = atoi(p);
								}
								if (i == 4) {
									mport = (atoi(p) << 8);
								}
								if (i == 5) {
									mport = (mport | atoi(p));
								}
								i++;
								p = strtok(NULL,",");
							}

							sprintf(buf,"%u.%u.%u.%u",ip[0],ip[1],ip[2],ip[3]);
							dSock = this->socks->Create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
							if (dSock == NULL) {
								this->error = TD_ERROR_CREATING_SOCKET;
								this->socks->Close(sock);
								return false;
							}
							if (!socks->Connect(dSock,buf,mport)) {
								this->error = TD_ERROR_CONNECTING;
								this->socks->Close(dSock);
								this->socks->Close(sock);
								return false;
							}

							sprintf(buf,"RETR %s\r\n",this->path);
							this->socks->Send(sock,buf);
							ln++;
						} else {
							this->error = TD_INVALID_RESPONSE;
							this->socks->Close(sock);
							return false;
						}
					}
					break;
				default:
					this->error = TD_INVALID_RESPONSE;
					this->socks->Close(sock);
					return false;
					break;
			}
			continue;
		}

		if (ln == 5) { // after RETR
			switch(atol(code)) {
				case 150:{
						char *p = strchr(data,'(');
						if (p) {
							p++;
							if (strchr(p,' ')) {
								strchr(p,' ')[0]=0;
							}
							if (strchr(p,')')) {
								strchr(p,')')[0]=0;
							}

							fullsize = atol(p);
							long left = fullsize;

							while (left && (n = this->socks->Recv(dSock,buf,16384)) > 0) {
								left -= n;
								got += n;
								if (fWriteTo->write(buf,n,fWriteTo) < n) {
									this->error = TD_FILE_WRITE_ERROR;
									if (dSock) { this->socks->Close(dSock); dSock=0; }
									this->socks->Close(sock);
									return false;
								}
								if (this->callback != NULL && !this->callback(got,fullsize,u_ptr)) {
									this->error = TD_CALLBACK_ABORT;
									if (dSock) { this->socks->Close(dSock); dSock=0; }
									this->socks->Close(sock);
									return false;
								}
							}

							if (dSock) { this->socks->Close(dSock); dSock=0; }
							ln++;
							/*
							this->socks->Close(sock);
							return true;
							*/
						}
					}
					break;
				default:
					this->error = TD_INVALID_RESPONSE;
					this->socks->Close(sock);
					return false;
					break;
			}
			continue;
		}

		if (ln == 6) { // after transfer complete
			switch(atol(code)) {
				case 226:
					sprintf(buf,"QUIT\r\n");
					this->socks->Send(sock,buf);
					if (dSock) { this->socks->Close(dSock); dSock=0; }
					this->socks->Close(sock);
					return true;
					break;
				default:
					this->error = TD_INVALID_RESPONSE;
					this->socks->Close(sock);
					return false;
					break;
			}
			continue;
		}

		this->error = TD_INVALID_RESPONSE;
		if (dSock) { this->socks->Close(dSock); dSock=0; }
		this->socks->Close(sock);
		return false;
	}

	if (dSock) { this->socks->Close(dSock); dSock=0; }
	this->socks->Close(sock);
	return false;
}

//#endif // !defined(NO_CURL)
