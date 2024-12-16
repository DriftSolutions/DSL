//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

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

size_t tdCurlWrite(void *ptr, size_t size, size_t nmemb, void *stream) {
	DSL_FILE * fp = (DSL_FILE *)stream;
	int64 ret = fp->write(ptr, size * nmemb, fp);
	if (ret <= 0) {
		return 0;
	}
	return size_t(ret) / size;
}

int tdCurlCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
	DSL_Download_CurlCallback * cb = (DSL_Download_CurlCallback *)clientp;
	return cb->callback((uint64)dlnow, (uint64)dltotal, cb->user_ptr) ? 0:1;
}

DSL_Download_Curl::DSL_Download_Curl(const string& url, DSL_Download_Callback callback, const string& user, const string& pass, void * user_ptr) {
	memset(&curlcb, 0, sizeof(curlcb));

	if (!curl_has_init) {
		//libCURL is not initialized
		error = TD_ERROR_INITIALIZING_LIB;
		return;
	}

	cHandle = curl_easy_init();
	if (cHandle == NULL) {
		error = TD_ERROR_CREATING_SOCKET;
		return;
	}

	//printf("RAW CURL GET: %s\n", url);

	curl_easy_setopt(cHandle, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(cHandle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(cHandle, CURLOPT_MAXREDIRS, 3);
	curl_easy_setopt(cHandle, CURLOPT_USERAGENT, "DSL HTTP Downloader Class (Mozilla)");
	curl_easy_setopt(cHandle, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(cHandle, CURLOPT_WRITEFUNCTION, tdCurlWrite);
	curl_easy_setopt(cHandle, CURLOPT_ERRORBUFFER, errstr);

	SetCallback(callback, user_ptr);
	SetURL(url);
	if (user.length() || pass.length()) {
		SetUserPass(user, pass);
	}
}

void DSL_Download_Curl::SetUserPass(const string& user, const string& pass) {
	if (cHandle != NULL) {
		if (user.length() || pass.length()) {
			curl_easy_setopt(cHandle, CURLOPT_USERPWD, mprintf("%s:%s", user.c_str(), pass.c_str()).c_str());
		} else {
			curl_easy_setopt(cHandle, CURLOPT_USERPWD, (char *)NULL);
		}
	}
}

DSL_Download_Curl::~DSL_Download_Curl() {
	if (cHandle != NULL) {
		curl_easy_cleanup(cHandle);
		cHandle = NULL;
	}
}

const char * DSL_Download_Curl::GetErrorString() {
	if (errstr[0]) {
		return errstr;
	}
	return DSL_Download_Base::GetErrorString();
}

//DSL_Download_Errors DSL_Download::GetError() { return this->error; }
bool DSL_Download_Curl::SetURL(const string& url) {
	if (cHandle) {
		return (curl_easy_setopt(cHandle, CURLOPT_URL, url.c_str()) == CURLE_OK);
	}
	return false;
}

void DSL_Download_Curl::SetCallback(DSL_Download_Callback callback, void * user_ptr) {
	if (cHandle) {
		memset(&curlcb, 0, sizeof(curlcb));
		if (callback != NULL) {
			curlcb.callback = callback;
			curlcb.user_ptr = user_ptr;
			curl_easy_setopt(cHandle, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(cHandle, CURLOPT_PROGRESSFUNCTION, tdCurlCallback);
			curl_easy_setopt(cHandle, CURLOPT_PROGRESSDATA, &curlcb);
		} else {
			curl_easy_setopt(cHandle, CURLOPT_NOPROGRESS, 1);
			curl_easy_setopt(cHandle, CURLOPT_PROGRESSFUNCTION, NULL);
			curl_easy_setopt(cHandle, CURLOPT_PROGRESSDATA, NULL);
		}
	}
}

void DSL_Download_Curl::SetTimeout(uint32 millisec) {
	if (cHandle) { curl_easy_setopt(cHandle, CURLOPT_TIMEOUT_MS, (unsigned long)millisec); }
}

void DSL_Download_Curl::FollowRedirects(bool follow) {
	if (cHandle) { curl_easy_setopt(cHandle, CURLOPT_MAXREDIRS, follow ? 3:0); }
}

void DSL_Download_Curl::SetUserAgent(const string& ua) {
	if (cHandle) { curl_easy_setopt(cHandle, CURLOPT_USERAGENT, ua.c_str()); }
}

void DSL_Download_Curl::SetProxy(const string& proxy) {
	if (cHandle) { curl_easy_setopt(cHandle, CURLOPT_PROXY, proxy.c_str()); }
}

CURLcode DSL_Download_Curl::SetOptStr(CURLoption option, const char * p) {
	if (cHandle) { return curl_easy_setopt(cHandle, option, p); }
	return CURLE_FAILED_INIT;
}

CURLcode DSL_Download_Curl::SetOptVoid(CURLoption option, void * p) {
	if (cHandle) { return curl_easy_setopt(cHandle, option, p); }
	return CURLE_FAILED_INIT;
}

CURLcode DSL_Download_Curl::SetOptLong(CURLoption option, long p) {
	if (cHandle) { return curl_easy_setopt(cHandle, option, p); }
	return CURLE_FAILED_INIT;
}

CURLcode DSL_Download_Curl::SetOptOff(CURLoption option, curl_off_t p) {
	if (cHandle) { return curl_easy_setopt(cHandle, option, p); }
	return CURLE_FAILED_INIT;
}

bool DSL_Download_Curl::Download(DSL_FILE * fWriteTo) {
	if (this->error != TD_NO_ERROR) { return false; }
	if (cHandle == NULL) {
		this->error = TD_ERROR_INITIALIZING_LIB;
		return false;
	}

	curl_easy_setopt(cHandle, CURLOPT_WRITEDATA, fWriteTo);

	CURLcode ret = curl_easy_perform(cHandle);
	switch (ret) {
		case CURLE_OK:
			error = TD_NO_ERROR;
			break;
		case CURLE_REMOTE_FILE_NOT_FOUND:
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

string curl_escapestring(const string& str, CURL * cHandle) {
	string ret;
#if LIBCURL_VERSION_MAJOR > 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR >= 82)
	// cHandle is ignored in newer versions
	char * tmp = curl_easy_escape(cHandle, str.c_str(), (int)str.length());
#else
	char * tmp = (cHandle != NULL) ? curl_easy_escape(cHandle, str.c_str(), (int)str.length()) : curl_escape(str.c_str(), (int)str.length());
#endif
	if (tmp != NULL) {
		ret = tmp;
		curl_free(tmp);
	}
	return ret;
}

string curl_http_build_query(const map<string, string>& values, CURL * cHandle) {
	stringstream url;
	bool first = true;
	for (auto& x : values) {
		if (first) {
			first = false;
		} else {
			url << "&";
		}
		url << curl_escapestring(x.first, cHandle) << "=" << curl_escapestring(x.second, cHandle);
	}
	return url.str();
}

#endif
