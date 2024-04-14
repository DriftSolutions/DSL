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

DSL_Download_Base::~DSL_Download_Base() {
}

DSL_Download_Errors DSL_Download_Base::GetError() { return this->error; }

static const char * DSL_Download_Error_Strings[TD_NUM_ERRORS] = {
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

const char * DSL_Download_Base::GetErrorString() {
	if (this->error >= TD_NUM_ERRORS) {
		static char unkerr[] = "Unkown Error";
		return unkerr;
	}
	return DSL_Download_Error_Strings[this->error];
}

bool DSL_Download_Base::Download(const string& SaveAs) {
	if (this->error != TD_NO_ERROR || SaveAs.empty()) { return false; }

	DSL_FILE * fp = RW_OpenFile(SaveAs.c_str(), "wb");
	if (fp == NULL) {
		this->error = TD_FILE_ACCESS;
		return false;
	}
	bool ret = Download(fp);
	fp->close(fp);
	return ret;
}

bool DSL_Download_Base::Download(FILE * fWriteTo) {
	if (this->error != TD_NO_ERROR || fWriteTo == NULL) { return false; }

	DSL_FILE * fp = RW_ConvertFile(fWriteTo, false);
	if (fp == NULL) {
		this->error = TD_FILE_ACCESS;
		return false;
	}
	bool ret = Download(fp);
	fp->close(fp);
	return ret;
}
