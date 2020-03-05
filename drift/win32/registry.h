//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef __REGISTRY_H__
#define __REGISTRY_H__

enum TR_VALUE_TYPE {
	TR_STRING,
	TR_LONG,
	TR_BINARY,
	TR_UNKNOWN
};

//string will be Unicode if used with Unicode APIs, otherwise not
struct TR_VALUE {
	TR_VALUE_TYPE type;
	unsigned long length;
	union {
		wchar_t * valWStr;
		char * valStr;
		BYTE * valBin;
		long valLong;
	};
};

typedef bool (*EV_CallbackA)(const char * name, TR_VALUE * value);
typedef bool (*EV_CallbackW)(const wchar_t * name, TR_VALUE * value);

class DSL_API_CLASS DSL_Registry {
public:

	bool EnumValuesCallbackW(HKEY hKey, const wchar_t * SubKey, EV_CallbackW callback);
	bool DeleteValueW(HKEY hKey, const wchar_t * SubKey, const wchar_t * value);
	bool SetValueW(HKEY hKey, const wchar_t * SubKey, const wchar_t * valname, TR_VALUE * value);
	bool GetValueW(HKEY hKey, const wchar_t * SubKey, const wchar_t * valname, TR_VALUE * value);

	bool EnumValuesCallbackA(HKEY hKey, const char * SubKey, EV_CallbackA callback);
	bool DeleteValueA(HKEY hKey, const char * SubKey, const char * value);
	bool SetValueA(HKEY hKey, const char * SubKey, const char * valname, TR_VALUE * value);
	bool GetValueA(HKEY hKey, const char * SubKey, const char * valname, TR_VALUE * value);

	void FreeValue(TR_VALUE * val);//this only needs to be done after GetValue
};

#if defined(UNICODE) || defined(_UNICODE)
#define EV_Callback EV_CallbackW
#define TREnumValuesCallback EnumValuesCallbackW
#define TRDeleteValue DeleteValueW
#define TRSetValue SetValueW
#define TRGetValue GetValueW
#define TRSTRFIELD valWStr
#else
#define EV_Callback EV_CallbackA
#define TREnumValuesCallback EnumValuesCallbackA
#define TRDeleteValue DeleteValueA
#define TRSetValue SetValueA
#define TRGetValue GetValueA
#define TRSTRFIELD valStr
#endif

#endif // __REGISTRY_H__
