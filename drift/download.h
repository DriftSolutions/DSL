//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_DOWNLOAD_H__
#define __DSL_DOWNLOAD_H__

#include <drift/rwops.h>
#include <drift/sockets3.h>

/**
 * \defgroup download HTTP/FTP File Transfer
 */

/** \addtogroup download
 * @{
 */

/**
 * Progress callback for the download class.
 * @param got How much of the fille has been received so far.
 * @param fullsize The full size of the file that is being downloaded. Will be 0 if the server doesn't give a Content-Length header.
 * @return true to continue download or false to abort.
 */
typedef bool (*DSL_Download_Callback)(uint64 got, uint64 fullsize, void * user_ptr);

/** \enum DSL_Download_Type
 * The type of download to perform.
 */
enum DSL_Download_Type {
	TD_HTTP, ///< HTTP
	TD_HTTPS, ///< HTTPS (needs ENABLE_CURL defined)
	TD_FTP ///< FTP
};

#define DSL_SHOULDRETRY(x) ((x < 5) & (x > 0))
/** \enum DSL_Download_Errors
 * Download error codes
 */
enum DSL_Download_Errors {
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

/** \class DSL_Download_Core
 * This base class shouldn't be used directly.
 * @sa DSL_Download_Curl
 * @sa DSL_Download_NoCurl
 */
class DSL_API_CLASS DSL_Download_Core {
protected:
	DSL_Download_Callback callback;
	void * u_ptr; ///< Pointer set by you and will be sent to your callback function for your own use
	DSL_Download_Errors error;
	virtual void privZero();
public:
#ifndef DOXYGEN_SKIP
	DSL_Download_Core();
	DSL_Download_Core(const char * url, DSL_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	DSL_Download_Core(DSL_Download_Type type, const char * host, int port, const char * path, DSL_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	virtual ~DSL_Download_Core();
#endif

	virtual bool Download(const char * SaveAs); ///< Begin download saving to file SaveAs
	virtual bool Download(FILE * fWriteTo); ///< Begin download saving to FILE stream
	/**
	 * Begin download saving to DSL_FILE stream.
	 * @sa DSL_FILE
	 */
	virtual bool Download(DSL_FILE * fWriteTo) = 0;

	virtual void SetTimeout(unsigned long millisec) = 0;
	virtual void SetUserAgent(const char * ua) = 0;
	virtual void FollowRedirects(bool follow) = 0;
	virtual void ErrorOnRedirects(bool error) = 0;

	virtual DSL_Download_Errors GetError();
	virtual const char * GetErrorString();
};

class DSL_API_CLASS DSL_Download_NoCurl: public DSL_Download_Core {
private:
	char host[128],path[512],user_agent[512];
	int port;
	bool followRedirects, errorOnRedirects;
	unsigned long timeo;
	DSL_Download_Type mode;
	//DSL_Download_Errors error;
	char user[128],pass[128];
	virtual void privZero();
	bool privDownloadHTTP(DSL_FILE * fWriteTo);
	bool privDownloadFTP(DSL_FILE * fWriteTo);
	DSL_Sockets3_Base * socks;
public:
	DSL_Download_NoCurl();
	DSL_Download_NoCurl(const char * url, DSL_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	DSL_Download_NoCurl(DSL_Download_Type type, const char * host, int port, const char * path, DSL_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	virtual ~DSL_Download_NoCurl();

	virtual bool Download(const char * SaveAs); ///< Begin download saving to file SaveAs
	virtual bool Download(FILE * fWriteTo); ///< Begin download saving to FILE stream
	/**
	 * Begin download saving to DSL_FILE stream.
	 * @sa DSL_FILE
	 */
	virtual bool Download(DSL_FILE * fWriteTo);

	virtual void SetTimeout(unsigned long millisec);
	virtual void SetUserAgent(const char * ua);
	virtual void FollowRedirects(bool follow);
	virtual void ErrorOnRedirects(bool error);

	//virtual DSL_Download_Errors GetError();
	//virtual const char * GetErrorString();
};

#if defined(ENABLE_CURL) || defined(DOXYGEN_SKIP)

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

#ifndef DOXYGEN_SKIP
struct DSL_Download_CurlCallback {
	DSL_Download_Callback callback;
	void * user_ptr;
};
#endif

class DSL_CURL_API_CLASS DSL_Download_Curl: public DSL_Download_Core {
private:
	virtual void privZero();
	CURL * cHandle;
	DSL_Download_CurlCallback curlcb;
	bool pCommonInit(const char * url=NULL, DSL_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);

public:
	DSL_Download_Curl();
	DSL_Download_Curl(const char * url, DSL_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	DSL_Download_Curl(DSL_Download_Type type, const char * host, int port, const char * path, DSL_Download_Callback callback=NULL, const char * user=NULL, const char * pass=NULL, void * user_ptr=NULL);
	virtual ~DSL_Download_Curl();

	virtual bool Download(const char * SaveAs); ///< Begin download saving to file SaveAs
	virtual bool Download(FILE * fWriteTo); ///< Begin download saving to FILE stream
	/**
	 * Begin download saving to DSL_FILE stream.
	 * @sa DSL_FILE
	 */
	virtual bool Download(DSL_FILE * fWriteTo);

	virtual void SetTimeout(unsigned long millisec);
	virtual void SetUserAgent(const char * ua);
	virtual void FollowRedirects(bool follow);
	virtual void ErrorOnRedirects(bool error);
};

DSL_API string curl_escapestring(string str);

#define DSL_Download DSL_Download_Curl
#else
#define DSL_Download DSL_Download_NoCurl
#endif

/**@}*/

#endif // __DSL_DOWNLOAD_H__
