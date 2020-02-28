//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef __TITUS_DOWNLOAD_H__
#define __TITUS_DOWNLOAD_H__

#include <drift/rwops.h>
#include <drift/sockets3.h>

typedef bool (*Titus_Download_Callback)(uint64 got, uint64 fullsize, void * user_ptr);
// fullsize will be 0 if the server doesn't give a Content-Length header

enum Titus_Download_Type {
	TD_HTTP,
	TD_HTTPS,
	TD_FTP
};

#define TITUS_SHOULDRETRY(x) ((x < 5) & (x > 0))
enum Titus_Download_Errors {
	TD_NO_ERROR,

	TD_FILE_ACCESS,
	TD_ERROR_CONNECTING,
	TD_INVALID_RESPONSE,
	TD_ERROR_CREATING_SOCKET,
	TD_ERROR_INITIALIZING_LIB,

	TD_FILE_WRITE_ERROR,
	TD_INVALID_URL,
	TD_CALLBACK_ABORT, // when the callback returns false, Download returns false and sets error to this
	TD_INVALID_PROTOCOL,
	TD_FILE_NOT_FOUND,
	TD_REDIRECT,
	TD_TIMEOUT,
	TD_BAD_USER_PASS,
	TD_NUM_ERRORS
};

class DSL_API_CLASS TitusDownloadCore {
protected:
	Titus_Download_Callback callback;
	void * u_ptr; // pointer set by you and will be sent to your callback function for your own use
	Titus_Download_Errors error;
	virtual void privZero();
public:
	TitusDownloadCore();
	TitusDownloadCore(const char * url, Titus_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	TitusDownloadCore(Titus_Download_Type type, const char * host, int port, const char * path, Titus_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	virtual ~TitusDownloadCore();

	virtual bool Download(const char * SaveAs); // begin download
	virtual bool Download(FILE * fWriteTo); // begin download
	virtual bool Download(TITUS_FILE * fWriteTo) = 0; // begin download

	virtual void SetTimeout(unsigned long millisec) = 0;
	virtual void SetUserAgent(const char * ua) = 0;
	virtual void FollowRedirects(bool follow) = 0;
	virtual void ErrorOnRedirects(bool error) = 0;

	virtual Titus_Download_Errors GetError();
	virtual const char * GetErrorString();
};

class DSL_API_CLASS TitusDownloadNoCurl: public TitusDownloadCore {
private:
	char host[128],path[512],user_agent[512];
	int port;
	bool followRedirects, errorOnRedirects;
	unsigned long timeo;
	Titus_Download_Type mode;
	//Titus_Download_Errors error;
	char user[128],pass[128];
	virtual void privZero();
	bool privDownloadHTTP(TITUS_FILE * fWriteTo);
	bool privDownloadFTP(TITUS_FILE * fWriteTo);
	Titus_Sockets3_Base * socks;
public:
	TitusDownloadNoCurl();
	TitusDownloadNoCurl(const char * url, Titus_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	TitusDownloadNoCurl(Titus_Download_Type type, const char * host, int port, const char * path, Titus_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	virtual ~TitusDownloadNoCurl();

	virtual bool Download(const char * SaveAs); // begin download
	virtual bool Download(FILE * fWriteTo); // begin download
	virtual bool Download(TITUS_FILE * fWriteTo); // begin download

	virtual void SetTimeout(unsigned long millisec);
	virtual void SetUserAgent(const char * ua);
	virtual void FollowRedirects(bool follow);
	virtual void ErrorOnRedirects(bool error);

	//virtual Titus_Download_Errors GetError();
	//virtual const char * GetErrorString();
};

#if defined(ENABLE_CURL)

#if defined(DSL_DLL) && defined(WIN32)
	#if defined(DSL_CURL_EXPORTS)
		#define DSL_CURL_API extern "C" __declspec(dllexport)
		#define DSL_CURL_API_CLASS __declspec(dllexport)
	#else
		#define DSL_CURL_API extern "C" __declspec(dllimport)
		#define DSL_CURL_API_CLASS __declspec(dllimport)
	#endif
#else
	#define DSL_CURL_API
	#define DSL_CURL_API_CLASS
#endif

struct TitusDownloadCurlCallback {
	Titus_Download_Callback callback;
	void * user_ptr;
};

class DSL_CURL_API_CLASS TitusDownloadCurl: public TitusDownloadCore {
private:
	virtual void privZero();
	CURL * cHandle;
	TitusDownloadCurlCallback curlcb;
	bool pCommonInit(const char * url=NULL, Titus_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);

public:
	TitusDownloadCurl();
	TitusDownloadCurl(const char * url, Titus_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	TitusDownloadCurl(Titus_Download_Type type, const char * host, int port, const char * path, Titus_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	virtual ~TitusDownloadCurl();

	virtual bool Download(TITUS_FILE * fWriteTo);
	virtual bool Download(const char * SaveAs); // begin download
	virtual bool Download(FILE * fWriteTo); // begin download

	virtual void SetTimeout(unsigned long millisec);
	virtual void SetUserAgent(const char * ua);
	virtual void FollowRedirects(bool follow);
	virtual void ErrorOnRedirects(bool error);
};

#define Titus_Download TitusDownloadCurl
#else
#define Titus_Download TitusDownloadNoCurl
#endif

#endif // __TITUS_DOWNLOAD_H__
