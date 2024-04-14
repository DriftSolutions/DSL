//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2024 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

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

class DSL_CURL_API_CLASS DSL_Download_Curl : public DSL_Download_Base {
private:
	CURL * cHandle = NULL;
	DSL_Download_CurlCallback curlcb;
	char errstr[CURL_ERROR_SIZE] = { 0 };
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
