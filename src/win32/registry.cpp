//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#if defined(_WIN32)
#include <drift/dslcore.h>
#include <drift/win32/registry.h>

//typedef bool (*EV_Callback)(char * name, char * value);

bool DSL_Registry::EnumValuesCallbackA(HKEY hKey, const char * SubKey, EV_CallbackA callback) {
	HKEY ko;
	if (RegOpenKeyExA(hKey,SubKey,0,KEY_READ|KEY_QUERY_VALUE,&ko) != NO_ERROR) {
		return false;
	}

	char name[32766]={0};
	DWORD i = 0, dName=sizeof(name),dType=0,dSize=0;
	DWORD dErr=0,*pLong=NULL;
	BYTE * pData=NULL;
	while (RegEnumValueA(ko,i,name,&dName,NULL,&dType,NULL,&dSize) == ERROR_SUCCESS) {
		dName=sizeof(name);
		pData = new BYTE[dSize];

		if ((dErr = RegEnumValueA(ko,i,name,&dName,NULL,&dType,pData,&dSize)) == ERROR_SUCCESS) {
			TR_VALUE tv;
			tv.length = dSize;
			switch(dType) {
				case REG_SZ:
				case REG_MULTI_SZ:
				case REG_EXPAND_SZ:
					tv.type = TR_STRING;
					tv.valStr = (char *)pData;
					break;
				case REG_BINARY:
					tv.type = TR_BINARY;
					tv.valBin = pData;
					break;
				case REG_DWORD:
					tv.type = TR_LONG;
					pLong = (DWORD *)pData;
					tv.valLong = *pLong;
					break;
				default:
					tv.type = TR_UNKNOWN;
					tv.valBin = pData;
			}
			callback(name,&tv);
		} else {
//			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,dErr,NULL,name,sizeof(name),NULL);
//			printf("Errored: %s\n",name);
			delete pData;
			break;
		}

		delete pData;
		pData=NULL;
		dName=sizeof(name);
		dType=0,dSize=0;
		name[0]=0;
		i++;
	}

	RegCloseKey(ko);
	return true;
};

bool DSL_Registry::DeleteValueA(HKEY hKey, const char * SubKey, const char * value) {
	HKEY ko;
	if (RegOpenKeyExA(hKey,SubKey,0,KEY_READ|KEY_WRITE,&ko) != NO_ERROR) {
		return false;
	}
	if (RegDeleteValueA(ko,value) == ERROR_SUCCESS) {
		RegCloseKey(ko);
		return true;
	} else {
		RegCloseKey(ko);
		return false;
	}
}

bool DSL_Registry::SetValueA(HKEY hKey, const char * SubKey, const char * valname, TR_VALUE * value) {
	HKEY ko;
	if (RegCreateKeyExA(hKey,SubKey,0,NULL,REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &ko, NULL) != NO_ERROR) {
		return false;
	}
	LONG ret=0;
	switch(value->type) {
		case TR_STRING:
			ret = RegSetValueExA(ko,valname,NULL,REG_SZ,(const BYTE *)value->valStr,value->length+1);
			break;
		case TR_LONG:{
				DWORD ll = value->valLong;
				ret = RegSetValueExA(ko,valname,NULL,REG_DWORD,(const unsigned char *)&ll,sizeof(DWORD));
			}
			break;
		case TR_BINARY:
		default:
			ret = RegSetValueExA(ko,valname,NULL,REG_BINARY,value->valBin,value->length);
			break;
	}
	if (ret == ERROR_SUCCESS) {
		RegCloseKey(ko);
		return true;
	} else {
		RegCloseKey(ko);
		return false;
	}
}

bool DSL_Registry::GetValueA(HKEY hKey, const char * SubKey, const char * valname, TR_VALUE * value) {
	HKEY ko;
	if (RegOpenKeyExA(hKey,SubKey,0,KEY_READ,&ko) != NO_ERROR) {
		return false;
	}
	memset(value, 0, sizeof(TR_VALUE));
	DWORD type;
	LONG ret = RegQueryValueExA(ko,valname,NULL,&type,NULL,&value->length);
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(ko);
		return false;
	}

	switch(type) {
		case REG_SZ:
			value->type = TR_STRING;
			value->length++;
			value->valStr = (char *)dsl_malloc(value->length);
			memset(value->valStr, 0, value->length);
			ret = RegQueryValueExA(ko,valname,NULL,&type,(unsigned char *)value->valStr,&value->length);
			break;
		case REG_DWORD:
			value->type = TR_LONG;
			DWORD ll;
			value->length = sizeof(ll);
			ret = RegQueryValueExA(ko,valname,NULL,&type,(unsigned char *)&ll,&value->length);
			value->valLong = ll;
			break;
		default:
			value->type = TR_BINARY;
			value->valBin = (unsigned char *)dsl_malloc(value->length);
			ret = RegQueryValueExA(ko,valname,NULL,&type,(unsigned char *)value->valBin,&value->length);
			break;
	}
	if (ret == ERROR_SUCCESS) {
		RegCloseKey(ko);
		return true;
	} else {
		RegCloseKey(ko);
		return false;
	}
}

void DSL_Registry::FreeValue(TR_VALUE * val) {
	if (val->type == TR_STRING || val->type == TR_BINARY) {
		if (val->valStr) {
			dsl_free(val->valStr);
			val->valStr = NULL;
		}
	}
}

#endif // defined(_WIN32)
