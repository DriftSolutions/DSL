//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#if defined(ENABLE_CURL)

#include <drift/dslcore.h>
#include <drift/download.h>

bool curl_has_init = false;

bool dsl_curl_init() {
	curl_has_init = (curl_global_init(CURL_GLOBAL_ALL) == 0);
	if (!curl_has_init) {
		printf("DSL: Error initializing libCURL, some functions will be disabled...\n");
	}
	return curl_has_init;
}

void dsl_curl_cleanup() {
	curl_has_init = false;
	curl_global_cleanup();
}

DSL_LIBRARY_FUNCTIONS dsl_curl_funcs = {
	false,
	DSL_OPTION_CURL,
	dsl_curl_init,
	dsl_curl_cleanup,
	NULL,
};
DSL_Library_Registerer dsl_curl_autoreg(dsl_curl_funcs);

void DSL_Download_Curl::privZero() {
	callback = NULL;
	error = TD_NO_ERROR;
	cHandle = NULL;
	memset(&curlcb, 0, sizeof(curlcb));
}

size_t tdCurlWrite(void *ptr, size_t size, size_t nmemb, void *stream) {
	DSL_FILE * fp = (DSL_FILE *)stream;
	return fp->write(ptr, size * nmemb, fp) / size;
}

int tdCurlCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
	DSL_Download_CurlCallback * cb = (DSL_Download_CurlCallback *)clientp;
	return cb->callback(dlnow, dltotal, cb->user_ptr) ? 0:1;
}

bool DSL_Download_Curl::pCommonInit(const char * url, DSL_Download_Callback callback, const char * user, const char * pass, void * user_ptr) {
	callback = callback;
	u_ptr = user_ptr;

	if (!curl_has_init) {
		//libCURL is not initialized
		error = TD_ERROR_INITIALIZING_LIB;
		return false;
	}
	/*
	tdcMutex.Lock();
	if (hasCurlInit != 1) {
		error = TD_ERROR_INITIALIZING_LIB;
		tdcMutex.Release();
		return false;
	}
	tdcMutex.Release();
	*/

	cHandle = curl_easy_init();
	if (cHandle == NULL) {
		error = TD_ERROR_CREATING_SOCKET;
		return false;
	}
	if (url == NULL) {
		error = TD_INVALID_URL;
		return false;
	}

	//printf("RAW CURL GET: %s\n", url);
	curl_easy_setopt(cHandle, CURLOPT_URL, url);
	curl_easy_setopt(cHandle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(cHandle, CURLOPT_MAXREDIRS, 3);
	curl_easy_setopt(cHandle, CURLOPT_USERAGENT, "DSL HTTP Downloader Class (Mozilla)");
	curl_easy_setopt(cHandle, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(cHandle, CURLOPT_WRITEFUNCTION, tdCurlWrite);
	if (callback) {
		curlcb.callback = callback;
		curlcb.user_ptr = u_ptr;
		curl_easy_setopt(cHandle, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(cHandle, CURLOPT_PROGRESSFUNCTION, tdCurlCallback);
		curl_easy_setopt(cHandle, CURLOPT_PROGRESSDATA, &curlcb);

	} else {
		curl_easy_setopt(cHandle, CURLOPT_NOPROGRESS, 1);
	}
	if (user || pass) {
		size_t len = 2;
		if (user) { len += strlen(user); }
		if (pass) { len += strlen(pass); }
		char * tmp = (char *)dsl_malloc(len);
		memset(tmp, 0, len);
		if (user) { strcat(tmp, user); }
		strcat(tmp, ":");
		if (pass) { strcat(tmp, pass); }
		curl_easy_setopt(cHandle, CURLOPT_USERPWD, tmp);
		dsl_free(tmp);
	}

	return true;
}

DSL_Download_Curl::DSL_Download_Curl() {
	privZero();
	pCommonInit();
}

DSL_Download_Curl::DSL_Download_Curl(const char * url, DSL_Download_Callback callback, const char * user, const char * pass, void * user_ptr) {
	privZero();
	pCommonInit(url, callback, user, pass, user_ptr);
}

char DSL_Download_Type_String[3][6] = {
	"http",
	"https",
	"ftp"
};

DSL_Download_Curl::DSL_Download_Curl(DSL_Download_Type type, const char * host, int port, const char * path, DSL_Download_Callback callback, const char * user, const char * pass, void * user_ptr) {
	privZero();
	if (type != TD_HTTP && type != TD_HTTPS && type != TD_FTP) {
		this->error = TD_INVALID_PROTOCOL;
		return;
	}
	char url[1024];
	memset(url, 0, sizeof(url));
	snprintf(url, sizeof(url)-1, "%s://%s:%d%s", DSL_Download_Type_String[type], host, port, path);
	pCommonInit(url, callback, user, pass, user_ptr);
}

DSL_Download_Curl::~DSL_Download_Curl() {
	if (cHandle) {
		curl_easy_cleanup(cHandle);
		cHandle = NULL;
	}
}

//DSL_Download_Errors DSL_Download::GetError() { return this->error; }

void DSL_Download_Curl::SetTimeout(unsigned long millisec) {
#if defined(CURLOPT_CONNECTTIMEOUT_MS)
	if (cHandle) { curl_easy_setopt(cHandle, CURLOPT_CONNECTTIMEOUT_MS, millisec); }
#else
	if (cHandle) { curl_easy_setopt(cHandle, CURLOPT_CONNECTTIMEOUT, millisec/1000); }
#endif
}

void DSL_Download_Curl::FollowRedirects(bool follow) {
	if (cHandle) { curl_easy_setopt(cHandle, CURLOPT_MAXREDIRS, follow ? 3:0); }
}

void DSL_Download_Curl::ErrorOnRedirects(bool error) {
	if (cHandle) { curl_easy_setopt(cHandle, CURLOPT_MAXREDIRS, error ? 0:3); }
}

void DSL_Download_Curl::SetUserAgent(const char * ua) {
	if (cHandle) { curl_easy_setopt(cHandle, CURLOPT_USERAGENT, ua); }
}

bool DSL_Download_Curl::Download(const char * SaveAs) {
	return DSL_Download_Core::Download(SaveAs);
}

bool DSL_Download_Curl::Download(FILE * fWriteTo) {
	return DSL_Download_Core::Download(fWriteTo);
}

bool DSL_Download_Curl::Download(DSL_FILE * fWriteTo) {
	if (this->error != TD_NO_ERROR || cHandle == NULL) { return false; }

	curl_easy_setopt(cHandle, CURLOPT_WRITEDATA, fWriteTo);

	CURLcode ret = curl_easy_perform(cHandle);
	switch (ret) {
#if defined(CURLE_REMOTE_FILE_NOT_FOUND)
		case CURLE_REMOTE_FILE_NOT_FOUND:
#endif
		case CURLE_TFTP_NOTFOUND:
			error = TD_FILE_NOT_FOUND;
			break;
		case CURLE_UNSUPPORTED_PROTOCOL:
			error = TD_INVALID_PROTOCOL;
			break;
		case CURLE_URL_MALFORMAT:
			error = TD_INVALID_URL;
			break;
		case CURLE_COULDNT_RESOLVE_PROXY:
		case CURLE_COULDNT_RESOLVE_HOST:
		case CURLE_COULDNT_CONNECT:
			error = TD_ERROR_CONNECTING;
			break;
		case CURLE_OPERATION_TIMEDOUT:
			error = TD_TIMEOUT;
			break;
		case CURLE_ABORTED_BY_CALLBACK:
			error = TD_CALLBACK_ABORT;
			break;
		case CURLE_TOO_MANY_REDIRECTS:
			error = TD_REDIRECT;
			break;
		default:
			error = TD_INVALID_RESPONSE;
			break;
	}

	return (ret == CURLE_OK) ? true:false;
}

#endif
