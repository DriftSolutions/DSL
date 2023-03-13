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
#include <drift/ini-reader.h>
#include <drift/Mutex.h>
#include <drift/GenLib.h>

DSL_Mutex  * iniMutex()
{
	static DSL_Mutex actualMutex;
	return &actualMutex;
}

char * DSL_CC Get_INI_String(const char * pszFilename, const char *pszSection, const char *pszItem, char *pszValue, size_t lValueBufferSize, const char * pszDefault) {
	AutoMutexPtr(iniMutex());

	char * ret = NULL;
	*pszValue = 0;

	FILE * hFile = fopen(pszFilename, "rb");
	if (hFile != NULL) {
		fseek(hFile, 0, SEEK_END);
		long lFileSize = ftell(hFile);
		char * cINIFile = (char *)dsl_malloc(lFileSize + 1);
		cINIFile[lFileSize] = 0;
		if (lFileSize > 0) {
			fseek(hFile, 0, SEEK_SET);
			if (fread(cINIFile, lFileSize, 1, hFile) == 1) {
				ret = Get_INI_String_Memory(cINIFile, pszSection, pszItem, pszValue, lValueBufferSize, pszDefault);
			}
		}
		fclose(hFile);
		dsl_free(cINIFile);
	}

	if (ret == NULL && pszDefault) {
		strlcpy(pszValue, pszDefault, lValueBufferSize);
		return pszValue;
	}

	return ret;
}

char * DSL_CC Get_INI_String_Memory(const char * data, const char *pszSection, const char *pszItem, char *pszValue, size_t lValueBufferSize, const char * pszDefault) {
	AutoMutexPtr(iniMutex());

	memset(pszValue, 0, lValueBufferSize);

	char * cfile = dsl_strdup(data);
	size_t len = strlen(cfile);
	str_replaceA(cfile, len+1, "\r", "");
	str_replaceA(cfile, len+1, "\t", " ");
	while (strstr(cfile, "  ")) {
		str_replaceA(cfile, len+1, "  ", " ");
	}
	len = strlen(cfile);

	bool insec   = false;
	char * line2 = NULL;
	char * line = strtok_r(cfile, "\n", &line2);
	while (line) {
		if (insec) {
			if (*line == '[' && strchr(line, ']')) {
				//a new section has opened, so stop looking for the correct item
				insec = false;
			} else {
				char * p = strchr(line, '=');
				if (p) {
					//trim up the item name and value for bad INI writers
					char * item = dsl_strdup(line);
					strchr(item, '=')[0] = 0;
					while (item[0] == ' ') {
						memmove(item, item+1, strlen(item));
					}
					while (item[strlen(item)-1] == ' ') {
						item[strlen(item)-1] = 0;
					}
					char * value = dsl_strdup(p+1);
					while (value[0] == ' ') {
						memmove(value, value+1, strlen(value));
					}
					while (value[strlen(value)-1] == ' ') {
						value[strlen(value)-1] = 0;
					}

					//is this my item?
					if (!stricmp(item, pszItem)) {
						strlcpy(pszValue, value, lValueBufferSize);
						dsl_free(item);
						dsl_free(value);
						dsl_free(cfile);
						return pszValue;
					}
					dsl_free(item);
					dsl_free(value);
				}
			}
		} else {
			if (*line == '[' && !strnicmp(line+1, pszSection, strlen(pszSection)) && line[strlen(pszSection)+1] == ']') {
				insec = true;
			}
		}
		line = strtok_r(NULL, "\n", &line2);
	}

	if (pszDefault) {
		strlcpy(pszValue, pszDefault, lValueBufferSize);
	} else {
		pszValue = NULL;
	}

	dsl_free(cfile);
	return pszValue;

}

int64 DSL_CC Get_INI_Int(const char *path, const char *section, const char *item, int64 iDefault) {
	AutoMutexPtr(iniMutex());

	char ivalue[64];
	char * ret = Get_INI_String(path, section, item, ivalue, sizeof(ivalue), NULL);
	if (ret) {
		return atoi64(ret);
	}
	return iDefault;
}

DSL_API int64 DSL_CC Get_INI_Int_Memory(const char * cINIFile, const char *section, const char *item, int64 iDefault) {
	AutoMutexPtr(iniMutex());

	char ivalue[64];
	char * ret = Get_INI_String_Memory(cINIFile, section, item, ivalue, sizeof(ivalue), NULL);
	if (ret) {
		return atoi64(ret);
	}
	return iDefault;
}

double DSL_CC Get_INI_Float(const char *path, const char *section, const char *item, double iDefault) {
	AutoMutexPtr(iniMutex());

	char ivalue[64];
	char * ret = Get_INI_String(path, section, item, ivalue, sizeof(ivalue), NULL);
	if (ret) {
		return atof(ret);
	}
	return iDefault;
}

DSL_API double DSL_CC Get_INI_Float_Memory(const char * cINIFile, const char *section, const char *item, double iDefault) {
	AutoMutexPtr(iniMutex());

	char ivalue[64];
	char * ret = Get_INI_String_Memory(cINIFile, section, item, ivalue, sizeof(ivalue), NULL);
	if (ret) {
		return atof(ret);
	}
	return iDefault;
}

int DSL_CC Write_INI_String(const char *pszPath, const char *pszSection, const char *pszItem, const char *pszValue) {
	FILE * hFile = fopen(pszPath, "rb");
	if (hFile == NULL) {
		//can't open, may not exist
		hFile = fopen(pszPath, "wb");
		if (hFile == NULL) {
			//nope, just an error in general
			return -1;
		}
		fprintf(hFile, "[%s]\r\n%s=%s\r\n", pszSection, pszItem, pszValue);
		fclose(hFile);
		return 0;
	}
	fseek(hFile, 0, SEEK_END);
	size_t len = (size_t)ftell(hFile);
	fseek(hFile, 0, SEEK_SET);
	char * cfile = (char *)dsl_malloc(len+1);
	cfile[len] = 0;
	if (fread(cfile, len, 1, hFile) != 1) {
		dsl_free(cfile);
		fclose(hFile);
		return -1;
	}
	fclose(hFile);
	hFile = fopen(pszPath, "wb");
	if (hFile == NULL) {
		dsl_free(cfile);
		return -1;
	}
	str_replaceA(cfile, len+1, "\r", "");
	str_replaceA(cfile, len+1, "\t", " ");
	while (strstr(cfile, "  ")) {
		str_replaceA(cfile, len+1, "  ", " ");
	}
	len = strlen(cfile);

	bool written = false;
	bool insec   = false;
	char * line2 = NULL;
	char * line = strtok_r(cfile, "\n", &line2);
	while (line) {
		if (insec) {
			if (*line == '[' && strchr(line, ']')) {
				//a new section has opened, so stop looking for the correct item
				if (!written) {
					//just append the value here before the new section opens
					fprintf(hFile, "%s=%s\r\n\r\n", pszItem, pszValue);
					written = true;
				}
				insec = false;
			} else {
				char * p = strchr(line, '=');
				if (p) {
					//trim up the item name and value for bad INI writers
					char * item = dsl_strdup(line);
					strchr(item, '=')[0] = 0;
					while (item[0] == ' ') {
						memmove(item, item+1, strlen(item));
					}
					while (item[strlen(item)-1] == ' ') {
						item[strlen(item)-1] = 0;
					}
					char * value = dsl_strdup(p+1);
					while (value[0] == ' ') {
						memmove(value, value+1, strlen(value));
					}
					while (value[strlen(value)-1] == ' ') {
						value[strlen(value)-1] = 0;
					}

					//is this my item?
					if (!stricmp(item, pszItem)) {
						fprintf(hFile, "%s=%s\r\n", pszItem, pszValue);
						written = true;
						dsl_free(item);
						dsl_free(value);
						line = strtok_r(NULL, "\n", &line2);
						continue;
					}
					dsl_free(item);
					dsl_free(value);
				}
			}
		} else {
			if (*line == '[' && !strnicmp(line+1, pszSection, strlen(pszSection)) && line[strlen(pszSection)+1] == ']') {
				insec = true;
			}
		}

		//printf("out: %s\r\n", line);
		fprintf(hFile, "%s\r\n", line);
		line = strtok_r(NULL, "\n", &line2);
	}

	if (written == false) {
		if (insec) {
			//EOF, but I am still in the correct section, no need to open a new one
			fprintf(hFile, "%s=%s\r\n", pszItem, pszValue);
		} else {
			// Section not found, create it add the section and write the value
			fprintf(hFile, "[%s]\r\n%s=%s\r\n", pszSection, pszItem, pszValue);
		}
	}

	fclose(hFile);
	dsl_free(cfile);
	return 0;
}

int DSL_CC Write_INI_Int(const char *pszPath, const char *pszSection, const char *pszItem, int64 iValue) {
	char value[64];
	sprintf(value, I64FMT, iValue);
	return Write_INI_String(pszPath, pszSection, pszItem, value);
}
