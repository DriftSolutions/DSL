//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
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

/** \enum DSL_Download_Errors
 * Download error codes
 */
enum DSL_Download_Errors : uint8 {
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

/** \class DSL_Download_Base
 * This base class shouldn't be used directly.
 * @sa DSL_Download_Curl
 * @sa DSL_Download_NoCurl
 */
class DSL_API_CLASS DSL_Download_Base {
protected:
	DSL_Download_Errors error = TD_INVALID_PROTOCOL;
public:
#ifndef DOXYGEN_SKIP
	virtual ~DSL_Download_Base();
#endif

	virtual bool SetURL(const string& url) = 0;
	virtual void SetCallback(DSL_Download_Callback callback, void * user_ptr = NULL) = 0;
	virtual void SetUserPass(const string& user, const string& pass) = 0;

	virtual bool Download(const string& SaveAs); ///< Begin download saving to file SaveAs
	virtual bool Download(FILE * fWriteTo); ///< Begin download saving to FILE stream
	/**
	 * Begin download saving to DSL_FILE stream.
	 * @sa DSL_FILE
	 */
	virtual bool Download(DSL_FILE * fWriteTo) = 0;
	/**
	 * Begin download saving to a DSL_BUFFER.
	 * @sa DSL_BUFFER
	virtual bool Download(DSL_BUFFER * fWriteTo) = 0;
	 */

	virtual void SetTimeout(uint32 millisec) = 0;
	virtual void SetUserAgent(const string& ua) = 0;
	virtual void FollowRedirects(bool follow) = 0;

	virtual DSL_Download_Errors GetError();
	virtual const char * GetErrorString();
};

class DSL_API_CLASS DSL_Download_NoCurl: public DSL_Download_Base {
private:
	string host, path, user_agent;
	uint16 port = 0;
	bool followRedirects = false;
	uint32 timeo = 0;
	string user, pass;
	DSL_Sockets3_Base * socks = NULL;
	DSL_Download_Callback callback = NULL;
	void * u_ptr = NULL; ///< Pointer set by you and will be sent to your callback function for your own use
public:
	DSL_Download_NoCurl(const string& url = "", DSL_Download_Callback callback = NULL, const string& user = "", const string& pass = "", void * user_ptr = NULL);
	virtual ~DSL_Download_NoCurl();

	virtual bool SetURL(const string& url);
	virtual bool GetURL(string& str);
	virtual void SetCallback(DSL_Download_Callback callback, void * user_ptr = NULL);
	virtual void SetUserPass(const string& user, const string& pass);

	/**
	 * Begin download saving to DSL_FILE stream.
	 * @sa DSL_FILE
	 */
	virtual bool Download(DSL_FILE * fWriteTo);
	virtual bool Download(FILE * fWriteTo) { return DSL_Download_Base::Download(fWriteTo); }
	virtual bool Download(const string& fSaveAs) { return DSL_Download_Base::Download(fSaveAs); }

	virtual void SetTimeout(uint32 millisec);
	virtual void SetUserAgent(const string& ua);
	virtual void FollowRedirects(bool follow = true);
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
	#define DSL_CURL_API DSL_API_VIS
	#define DSL_CURL_API_CLASS DSL_API_VIS
#endif

#ifndef DOXYGEN_SKIP
struct DSL_Download_CurlCallback {
	DSL_Download_Callback callback;
	void * user_ptr;
};
#endif

class DSL_CURL_API_CLASS DSL_Download_Curl: public DSL_Download_Base {
private:
	CURL * cHandle = NULL;
	DSL_Download_CurlCallback curlcb;
	char errstr[CURL_ERROR_SIZE] = {0};
public:
	DSL_Download_Curl(const string& url = "", DSL_Download_Callback callback = NULL, const string& user = "", const string& pass = "", void * user_ptr = NULL);
	virtual ~DSL_Download_Curl();

	virtual bool SetURL(const string& url);
	virtual void SetCallback(DSL_Download_Callback callback, void * user_ptr = NULL);
	virtual void SetUserPass(const string& user, const string& pass);

	/**
	 * Begin download saving to DSL_FILE stream.
	 * @sa DSL_FILE
	 */
	virtual bool Download(DSL_FILE * fWriteTo);
	virtual bool Download(FILE * fWriteTo) { return DSL_Download_Base::Download(fWriteTo); }
	virtual bool Download(const string& fSaveAs) { return DSL_Download_Base::Download(fSaveAs); }

	virtual void SetTimeout(uint32 millisec);
	virtual void SetUserAgent(const string& ua);
	virtual void FollowRedirects(bool follow = true);
	virtual void SetProxy(const string& proxy); /// See https://curl.se/libcurl/c/CURLOPT_PROXY.html
	virtual CURLcode SetOptStr(CURLoption option, const char * p);
	virtual CURLcode SetOptVoid(CURLoption option, void * p);
	virtual CURLcode SetOptLong(CURLoption option, long p);
	virtual CURLcode SetOptOff(CURLoption option, curl_off_t p);

	virtual const char * GetErrorString(); /// Return cURL-specific error message when possible, falling back to the defaults from DSL_Download_Base
};

DSL_CURL_API_CLASS string curl_escapestring(const string& str);

#define DSL_Download DSL_Download_Curl
#else
#define DSL_Download DSL_Download_NoCurl
#endif

/**@}*/

#endif // __DSL_DOWNLOAD_H__
