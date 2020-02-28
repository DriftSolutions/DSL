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

void TitusDownloadCore::privZero() {
	callback = NULL;
	u_ptr = NULL;
	error = TD_NO_ERROR;
}

TitusDownloadCore::TitusDownloadCore() {
	this->privZero();
}

TitusDownloadCore::TitusDownloadCore(const char * url, Titus_Download_Callback callback, const char * user, const char * pass, void * user_ptr) {
	this->privZero();
}

TitusDownloadCore::TitusDownloadCore(Titus_Download_Type type, const char * host, int port, const char * path, Titus_Download_Callback callback, const char * user, const char * pass, void * user_ptr) {
	this->privZero();
}

TitusDownloadCore::~TitusDownloadCore() {
}

Titus_Download_Errors TitusDownloadCore::GetError() { return this->error; }

char Titus_Download_Error_Strings[TD_NUM_ERRORS][56] = {
	"No Error",

	"File Access Error",
	"Error Connecting to Host",
	"Invalid Response",
	"Error Creating Socket",
	"Error initializing libCURL",

	"Error writing file",
	"Invalid URL",
	"Callback Abort",
	"Invalid Protocol",
	"File Not Found",
	"Server tried to redirect, but FollowRedirects is Off",
	"Transfer Timed Out",
	"Authorization Needed or Bad Username/Password"
};

const char * TitusDownloadCore::GetErrorString() {
	if (this->error >= TD_NUM_ERRORS) {
		static char unkerr[] = "Unkown Error";
		return unkerr;
	}
	return Titus_Download_Error_Strings[this->error];
}

bool TitusDownloadCore::Download(const char * SaveAs) {
	if (this->error != TD_NO_ERROR) { return false; }

	TITUS_FILE * fp = RW_OpenFile(SaveAs, "wb");
	if (fp == NULL) {
		this->error = TD_FILE_ACCESS;
		return false;
	}
	bool ret = this->Download(fp);
	fp->close(fp);
	return ret;
}

bool TitusDownloadCore::Download(FILE * fWriteTo) {
	if (this->error != TD_NO_ERROR) { return false; }

	TITUS_FILE * fp = RW_ConvertFile(fWriteTo, false);
	bool ret = this->Download(fp);
	fp->close(fp);
	return ret;
}
