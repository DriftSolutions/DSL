//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef _WIN32
#define NOMINMAX
#endif
#include <drift/dslcore.h>
#include <drift/GenLib.h>
#include <drift/Mutex.h>
#include <drift/rwops.h>
#include <algorithm>

#if defined(WIN32)
time_t filetime_2_time_t(FILETIME ft) {
	ULARGE_INTEGER tmp;
	tmp.LowPart = ft.dwLowDateTime;
	tmp.HighPart = ft.dwHighDateTime;
	uint64 ret = (tmp.QuadPart - 0x19DB1DED53E8000) / 10000000;
	return ret;
}
#endif

StrTokenizer::StrTokenizer(char * str, char separater, bool do_strdup) {
	b_strdup = do_strdup;
	if (b_strdup) {
	 	string = dsl_strdup(str);
	} else {
		string = str;
	}
	sep = separater;
	sep_str[0] = sep;

	bool ends_in_sep = false;
	if (string[strlen(string) - 1] == sep) {
		ends_in_sep = true;
	}

	num_tokens = 0;
	tokens = NULL;
	char * p2 = NULL;
	char * p = strtok_r(string, sep_str, &p2);
	while (p) {
		num_tokens++;
		tokens = (char **)dsl_realloc(tokens, sizeof(char *) * num_tokens);
		tokens[num_tokens - 1] = p;
		p = strtok_r(NULL, sep_str, &p2);
	}
	if (ends_in_sep) {
		num_tokens++;
		tokens = (char **)dsl_realloc(tokens, sizeof(char *) * num_tokens);
		tokens[num_tokens - 1] = p2;
	}

	//printf("numtok: %u\n", num_tokens);
}

StrTokenizer::~StrTokenizer() {
	if (b_strdup) {
		dsl_free(string);
	}
	dsl_freenn(tokens);
}

size_t StrTokenizer::NumTok() {
	return num_tokens;
}

char * StrTokenizer::GetTok(size_t first, size_t last) {

	size_t lSize=1;
	char * ret = (char *)dsl_malloc(lSize);
	ret[0]=0;

	first = clamp<size_t>(first, 1, num_tokens);
	last = clamp<size_t>(last, 1, num_tokens);
	for (size_t i = (first - 1); i < last; i++) {
		if (lSize > 1) {
			lSize += strlen(tokens[i]) + 1; // string + separater
			ret = (char *)dsl_realloc(ret, lSize);
			strlcat(ret, sep_str, lSize);
			strlcat(ret, tokens[i], lSize);
		} else {
			lSize += strlen(tokens[i]);
			ret = (char *)dsl_realloc(ret, lSize);
			strlcpy(ret, tokens[i], lSize);
		}
	}

	return ret;
}

char * StrTokenizer::GetSingleTok(size_t num) {
	return GetTok(num, num);
};

void StrTokenizer::FreeString(char * buf) {
	dsl_free(buf);
}

std::string StrTokenizer::stdGetTok(size_t first, size_t last) {
	//size_t num=0;
	std::string ret;

	first = clamp<size_t>(first, 1, num_tokens);
	last = clamp<size_t>(last, 1, num_tokens);
	for (size_t i = (first - 1); i < last; i++) {
		if (ret.length()) {
			ret += sep_str;
			ret += tokens[i];
		} else {
			ret = tokens[i];
		}
	}

	return ret;
}

std::string StrTokenizer::stdGetSingleTok(size_t num) {
	return stdGetTok(num, num);
};

char * DSL_CC bin2hex(const uint8_t * data, size_t datalen, char * out, size_t outsize) {
	unsigned short i;
	unsigned char j;

	if (outsize < (datalen * 2) + 1) {
		if (outsize > 0) {
			out[0] = 0;
		}
		return NULL;
	}

	//uint8 * data = (uint8 *)dsl_malloc(datalen);
	//memcpy(data, tdata, datalen);

	for (i = 0; i < datalen; i++) {
		j = (data[i] >> 4) & 0xf;
		if (j <= 9) {
			out[i * 2] = (j + '0');
		} else {
			out[i * 2] = (j + 'a' - 10);
		}
		j = data[i] & 0xf;
		if (j <= 9) {
			out[i * 2 + 1] = (j + '0');
		} else {
			out[i * 2 + 1] = (j + 'a' - 10);
		}
	};
	out[datalen * 2] = 0;
	//dsl_free(data);
	return out;
}

string DSL_CC bin2hex(const uint8_t * data, size_t datalen) {
	string ret;
	size_t len = (datalen * 2) + 1;
	char * tmp = (char *)dsl_malloc(len);
	if (bin2hex(data, datalen, tmp, len)) {
		ret = tmp;
	}
	dsl_free(tmp);
	return ret;
}

inline uint8 hex_digit_value(uint8 c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'a' && c <= 'f') {
		return (c - 'a') + 10;
	} else if (c >= 'A' && c <= 'F') {
		return (c - 'A') + 10;
	}
	return 0xFF;
}

bool DSL_CC hex2bin(const char * in, size_t len, uint8 * out, size_t outsize) {
	if ((len % 2) == 0 && outsize >= len / 2) {
		const char * p = in;
		for (size_t i = 0; i < len / 2 && *p != 0; i++, out++) {
			uint8 val = hex_digit_value(*p++);
			if (val == 0xFF) { return false; }
			*out = (val << 4) & 0xF0;
			val = hex_digit_value(*p++);
			if (val == 0xFF) { return false; }
			*out |= val;
		}
		return true;
	}

	memset(out, 0, outsize);
	return false;
}

bool DSL_CC hex2bin(const char * in, uint8 * out, size_t outsize) {
	size_t len = strlen(in);
	return hex2bin(in, len, out, outsize);
}

bool DSL_CC hex2bin(const string str, vector<uint8_t>& bin) {
	if (str.length() % 2 != 0) {
		return false;
	}
	size_t len = str.length() / 2;
	bin.resize(len);
	return hex2bin(str.c_str(), str.length(), bin.data(), len);
}

void DSL_CC PrintData(FILE * fp, const uint8 * ptr, size_t len) {
	unsigned int step = 0;
	for (size_t beg = 0; beg < len; beg += 16) {
		size_t tostep = ((len - beg) >= 16) ? 16 : (len - beg);

		fprintf(fp, "%08zxh: ", beg);

		for (step = 0; step < tostep; step++) {
			fprintf(fp, "%02X ", ptr[beg + step] & 0xFF);
		}
		while (step < 16) {
			fprintf(fp, "   ");
			step++;
		}
		fprintf(fp, " ; ");
		for (step = 0; step < tostep; step++) {
			char c = ptr[beg + step] & 0xFF;
			if (c < 32) { c = '.'; }
			fprintf(fp, "%c", c);
		}
		while (step < 16) {
			fprintf(fp, " ");
			step++;
		}

		fprintf(fp, "\n");
	}
	fflush(fp);
}

const char * DSL_CC nopathA(const char *fn) {
#ifdef WIN32
	const char *p = strrchr(fn,'\\');
	if (!p) { p = strrchr(fn,'/'); }
#else
	const char *p = strrchr(fn,'/');
	if (!p) { p = strrchr(fn,'\\'); }
#endif
	if (p) { return ++p; }
	return fn;
}

char * DSL_CC nopathA(char *fn) {
#ifdef WIN32
	char *p = strrchr(fn,'\\');
	if (!p) { p = strrchr(fn,'/'); }
#else
	char *p = strrchr(fn,'/');
	if (!p) { p = strrchr(fn,'\\'); }
#endif
	if (p) { return ++p; }
	return fn;
}

const wchar_t * DSL_CC nopathW(const wchar_t *fn) {
#ifdef WIN32
	const wchar_t *p = wcsrchr(fn,'\\');
	if (!p) { p = wcsrchr(fn,'/'); }
#else
	const wchar_t *p = wcsrchr(fn,'/');
	if (!p) { p = wcsrchr(fn,'\\'); }
#endif
	if (p) { return ++p; }
	return fn;
}

wchar_t * DSL_CC nopathW(wchar_t *fn) {
#ifdef WIN32
	wchar_t *p = wcsrchr(fn,'\\');
	if (!p) { p = wcsrchr(fn,'/'); }
#else
	wchar_t *p = wcsrchr(fn,'/');
	if (!p) { p = wcsrchr(fn,'\\'); }
#endif
	if (p) { return ++p; }
	return fn;
}

DSL_API char * DSL_CC strtrim(char *buf, const char * trim, uint8 sides) {
	size_t i=0;
	size_t len = strlen(trim);
	if (sides & TRIM_LEFT) {
		size_t n = strspn(buf, trim);
		if (n) {
			memmove(buf, buf + n, strlen(buf) - n + 1);
		}
	}
	if ((sides & TRIM_RIGHT) && buf[0]) {
		char * p = &buf[strlen(buf)-1];
		for (i=0; i < len; i++) {
			if (*p == trim[i]) {
				*p = 0;
				p--;
				i=-1;
			}
		}
	}
	return buf;
}

#if defined(WIN32)
char * DSL_CC strtok_r(char *string, const char * delimiters, char ** save_str) {
  //char *token;

	if (string == NULL) {
		string = *save_str;
	}

	string += strspn (string, delimiters);
	if (*string == 0) {
		*save_str = string;
		return NULL;
	}

	/* Find the end of the token.  */
	char * tok = string;
	string = strpbrk (tok, delimiters);
	if (string == NULL) {
		*save_str = strchr(tok, 0);
	} else {
		*string = 0;
		*save_str = string + 1;
	}
	return tok;
}

const char * DSL_CC stristr(const char * haystack, const char * needle) {
	size_t len = strlen(needle);
	while (*haystack) {
		if (toupper(*haystack) == toupper(*needle)) {
			if (strnicmp(haystack, needle, len) == 0) {
				return haystack;
			}
		}
		haystack++;
	}
	return NULL;
}

char * DSL_CC stristr(char * haystack, const char * needle) {
	size_t len = strlen(needle);
	while (*haystack) {
		if (toupper(*haystack) == toupper(*needle)) {
			if (strnicmp(haystack, needle, len) == 0) {
				return haystack;
			}
		}
		haystack++;
	}
	return NULL;
}

#else

char * DSL_CC strlwr(char *str) {
	for(char * p = str; *p; p++) {
		*p = tolower((unsigned char)*p);
	}
	return str;
}

#endif

bool DSL_CC dsl_mkdir_r(const char * fullpath, int mode) {
	if (fullpath == NULL) {
		return false;
	}

	char * newdir = dsl_strdup(fullpath);
	char * p = newdir;
#ifdef WIN32
	char * q = strchr(p, ':');
	if (q != NULL) {
		// skip drive letter
		p = q + 1;
	}
#endif
	while (*p == PATH_SEP) {
		//skip any initial slashes (Linux path or network/UNC path on Windows)
		p++;
	}
	if (newdir[strlen(newdir) - 1] == PATH_SEP) {
		//trim off any trailing slashes
		newdir[strlen(newdir) - 1] = 0;
	}
	if (access(newdir, F_OK) == 0) {
		dsl_free(newdir);
		return true;
	}

	char * tmp = dsl_strdup(fullpath);
	while ((p = strchr(p, PATH_SEP)) != NULL) {
		size_t len = p - newdir;
		strncpy(tmp, newdir, len);
		tmp[len] = 0;
		tmp = tmp;
		if (access(tmp, F_OK) != 0) {
			dsl_mkdir(tmp, mode);
		}
		p++;
	}
	bool ret = (dsl_mkdir(newdir, mode) == 0 || errno == EEXIST);
	dsl_free(tmp);
	dsl_free(newdir);
	return ret;
}

bool DSL_CC strempty(const char * p) {
	if (p == NULL) { return true; }
	return (*p == 0) ? true:false;
}

int64 DSL_CC dsl_clamp(int64 val, int64 vMin, int64 vMax) {
	return std::min(vMax, std::max(vMin, val));
}

int DSL_CC wildcmp(const char *wild, const char *string) {
  // Written by Jack Handy - <A href="mailto:jakkhandy@hotmail.com">jakkhandy@hotmail.com</A>
  // He posted it on codeproject with no explicit license, assuming public domain: https://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing
  // return 0 for no match, other for match
  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

int DSL_CC wildicmp(const char *wild, const char *string) {
  // Written by Jack Handy - <A href="mailto:jakkhandy@hotmail.com">jakkhandy@hotmail.com</A>
  // He posted it on codeproject with no explicit license, assuming public domain: https://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing
  // return 0 for no match, other for match
  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((tolower(*wild) != tolower(*string)) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((tolower(*wild) == tolower(*string)) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

int DSL_CC wildicmp_multi(const char *wild, const char *string, const char *sep) {
	int ret = 0;
	char * tmp = dsl_strdup((char *)wild);
	char * p2 = NULL;
	char * p = strtok_r(tmp, sep, &p2);
	while (p) {
		ret = wildicmp(p, string);
		if (ret != 0) { break; }
		p = strtok_r(NULL, sep, &p2);
	}
	dsl_free(tmp);
	return ret;
}

int64 DSL_CC fseek64(FILE * fp, int64 offset, int whence) {
#if defined(WIN32)
	return _fseeki64(fp, offset, whence);
#else
	return fseeko(fp, offset, whence);
#endif
}

int64 DSL_CC ftell64(FILE * fp) {
#if defined(WIN32)
	return _ftelli64(fp);
#else
	return ftello(fp);
#endif
}

int64 DSL_CC filesize(const char * fn) {
	struct stat64 st;
	if (stat64(fn, &st) == 0) {
		return st.st_size;
	}
	return -1;
}

#if defined(WIN32)
int DSL_CC truncate(const char * fn, int64 size) {
	int fd = open(fn, O_RDWR);
	if (fd != -1) {
		int ret = ftruncate(fd, size);
		close(fd);
		return ret;
	}
	return -1;
}
#endif

char * DSL_CC escapeshellarg(const char * p, char * out, size_t outSize) {
	memset(out, 0, outSize);

	outSize -= 3; // sub for potential final escaped char, final ", and null term

	strlcpy(out, "\"", outSize);
	char *x = out + 1;
	size_t len = 1;
	while (*p && len < outSize) {
		if (*p == '"') {
			*x = '\\';
			x++;
			len++;
		}
		*x = *p;
		p++;
		x++;
		len++;
	}
	strlcat(out, "\"", outSize);
	return out;
}

string DSL_CC escapeshellarg(const string& str) {
	string ret = "\"";
	const char *p = str.c_str();
	while (*p != 0) {
		if (*p == '"') {
			ret += '\\';
		}
		ret += *p;
		p++;
	}
	ret += "\"";
	return ret;
}

int DSL_CC str_replaceA(char *Str, size_t BufSize, const char *FindStr, const char *ReplStr) {
	char *p = NULL;
	int ret = 0;
	size_t OldLen = strlen(FindStr);
	size_t NewLen = strlen(ReplStr);
	while ((p = strstr(Str, FindStr))) {
		if ((strlen(Str) + NewLen - OldLen) > BufSize) {
			break;
		}
		if (NewLen != OldLen) {
			memmove(p + NewLen, p + OldLen, strlen(p+OldLen)+1);
		}
		memcpy(p, ReplStr, NewLen);
		Str = p+NewLen;
		ret++;
	}
	return ret;
}

#if !defined(FREEBSD)
int DSL_CC str_replaceW(wchar_t * Str, size_t BufSize, wchar_t *FindStr, wchar_t *ReplStr) {
	wchar_t * p, *End, *Begin;
	size_t len=0;
	wchar_t fmt[32];
	int ret=0;
	size_t OldLen = wcslen(FindStr);
	size_t NewLen = wcslen(ReplStr);
	while ((p = wcsstr(Str, FindStr))) {
		if ((wcslen(Str) + NewLen - OldLen + 1) > BufSize) {
			break;
		}
		End = dsl_wcsdup(p+OldLen);
		Begin = dsl_wcsdup(Str);
		len = p - Str;
		snwprintf(fmt, 32, L"%%.%zus%%s%%s", len);
		snwprintf(Str, BufSize, fmt, Begin, ReplStr, End);
		dsl_free(End);
		dsl_free(Begin);
		ret++;
	}
	return ret;
}
#endif

string DSL_CC str_replaceA(string str, string FindStr, string ReplStr) {
	size_t pos;
	while ((pos = str.find(FindStr)) != str.npos) {
		str.replace(pos, FindStr.length(), ReplStr);
	}
	return str;
}

#if !defined(FREEBSD)
wstring DSL_CC str_replaceW(wstring str, wstring FindStr, wstring ReplStr) {
	size_t pos;
	while ((pos = str.find(FindStr)) != str.npos) {
		str.replace(pos, FindStr.length(), ReplStr);
	}
	return str;
}
#endif // FREEBSD

char * DSL_CC GetUserConfigFolderA(const char * name) {
	char buf[MAX_PATH]={0};
#if defined(WIN32)
	char * p = getenv("APPDATA");
	if (p == NULL || *p == 0) {
		p = getenv("USERPROFILE");
	}
	if (p == NULL || *p == 0) {
		GetModuleFileNameA(NULL, buf, sizeof(buf));
		char * q = strrchr(buf, '\\');
		if (q) {
			*q = 0;
		}
	} else {
		sstrcpy(buf, p);
	}
#else
	char * p = getenv("HOME");
	if (p && *p) {
		sstrcpy(buf, p);
	} else if (getcwd(buf, sizeof(buf)) == NULL) {
		strcpy(buf, "./");
	}
#endif
	if (buf[strlen(buf)-1] != PATH_SEP) {
		sstrcat(buf, PATH_SEPS);
	}
#if !defined(WIN32)
	sstrcat(buf, ".");
#endif
	sstrcat(buf, name);
	struct stat st;
	if (stat(buf, &st) != 0) {
		dsl_mkdir(buf, 0700);
	}
	strcat(buf, PATH_SEPS);
	return dsl_strdup(buf);
}

string DSL_CC GetUserConfigFolderA(const string& name) {
	char * tmp = GetUserConfigFolderA(name.c_str());
	string ret = tmp;
	dsl_free(tmp);
	return ret;
}

char * DSL_CC GetUserConfigFileA(const char * name, const char * fn) {
	char * dir = GetUserConfigFolderA(name);
	char * ret = dsl_mprintf("%s%s", dir, fn);
	dsl_free(dir);
	return ret;
}

string DSL_CC GetUserConfigFileA(const string& name, const string& fn) {
	char * dir = GetUserConfigFolderA(name.c_str());
	string ret = mprintf("%s%s", dir, fn.c_str());
	dsl_free(dir);
	return ret;
}

wchar_t * DSL_CC GetUserConfigFolderW(const wchar_t * name) {
	wchar_t buf[MAX_PATH]={0};
#if defined(WIN32)
	wchar_t * p = wgetenv(L"APPDATA");
	if (!p || !wcslen(p)) {
		p = wgetenv(L"USERPROFILE");
	}
	if (!p || !wcslen(p)) {
		GetModuleFileNameW(NULL, buf, sizeof(buf));
		wchar_t * q = wcsrchr(buf, '\\');
		if (q) {
			q[1]=0;
		}
	} else {
		wcscpy(buf, p);
	}
#else
/*
	wchar_t * p = getenv("HOME");
	if (p && *p) {
		wcscpy(buf, p);
	} else {
		getcwd(buf, sizeof(buf)/sizeof(wchar_t));
	}
	*/
	wcscpy(buf, L".");
#endif
	if (buf[wcslen(buf)-1] != PATH_SEP) {
		wcscat(buf, WPATH_SEPS);
	}
	return dsl_wcsdup(buf);
}

wchar_t * DSL_CC GetUserConfigFileW(const wchar_t * name, const wchar_t * fn) {
	wchar_t * dir = GetUserConfigFolderW(name);
	wchar_t * ret = dsl_wmprintf(L"%s%s", dir, fn);
	dsl_free(dir);
	return ret;
}

char * DSL_CC GetUserDocumentsFolderA(const char * name) {
	char buf[MAX_PATH] = { 0 };
#if defined(WIN32)
	char * p = getenv("USERPROFILE");
	if (p == NULL || *p == 0) {
		GetModuleFileNameA(NULL, buf, sizeof(buf));
		char * q = strrchr(buf, '\\');
		if (q) {
			*q = 0;
		}
	} else {
		strlcpy(buf, p, sizeof(buf));
	}
#else
	char * p = getenv("HOME");
	if (p && *p) {
		strlcpy(buf, p, sizeof(buf));
	} else if (getcwd(buf, sizeof(buf)) == NULL) {
		strcpy(buf, "./");
	}
#endif
	if (buf[strlen(buf) - 1] != PATH_SEP) {
		sstrcat(buf, PATH_SEPS);
	}
	sstrcat(buf, "Documents" PATH_SEPS "");
	sstrcat(buf, name);
	struct stat st;
	if (stat(buf, &st) != 0) {
		dsl_mkdir(buf, 0700);
	}
	strcat(buf, PATH_SEPS);
	return dsl_strdup(buf);
}

#if !defined(WIN32)
static timeval tv_start = {0,0};
int genlib_gettickcount_i = gettimeofday(&tv_start, NULL);
//static timeval tv_start = {0,0};

uint32 GetTickCount() {
	if (!tv_start.tv_sec) {
		gettimeofday (&tv_start, NULL);
		tv_start.tv_usec = 0;
	}
	timeval tv;
	gettimeofday (&tv, NULL);
	unsigned long ret = (tv.tv_sec - tv_start.tv_sec) * 1000;
	ret += (tv.tv_usec / 1000);
	return ret;
}
#endif

DSL_API char * DSL_CC tcstombsA(const char * str) {
	return dsl_strdup(str);
}

DSL_API char * DSL_CC tcstombsW(const wchar_t * str) {
	size_t len = wcstombs(NULL, str, 0);
	if (len == (size_t)-1) {
		return dsl_strdup("ERROR in wcstombs");
	}
	char * buf = (char *)dsl_malloc(len+1);
	buf[len] = 0;
	wcstombs(buf, str, len);
	return buf;
};

DSL_API wchar_t * DSL_CC tcstowcsW(const wchar_t * str) {
	return dsl_wcsdup(str);
}
DSL_API wchar_t * DSL_CC tcstowcsA(const char * str) {
	size_t len = mbstowcs(NULL, str, 0);
	if (len == (size_t)-1) {
		return dsl_wcsdup(L"ERROR in mbstowcs");
	}
	size_t llen = (len+1)*sizeof(wchar_t);
	wchar_t * buf = (wchar_t *)dsl_malloc(llen);
	memset(buf, 0, llen);
	mbstowcs(buf, str, len);
	return buf;
};

DSL_API char * DSL_CC mbstotcsA(const char * str) {
	return dsl_strdup(str);
}
DSL_API char * DSL_CC wcstotcsA(const wchar_t * str) {
	size_t len = wcstombs(NULL, str, 0);
	if (len == (size_t)-1) {
		return dsl_strdup("ERROR in wcstombs");
	}
	char * buf = (char *)dsl_malloc(len+1);
	buf[len] = 0;
	wcstombs(buf, str, len);
	return buf;
}

DSL_API wchar_t * DSL_CC mbstotcsW(const char * str) {
	size_t len = mbstowcs(NULL, str, 0);
	if (len == (size_t)-1) {
		return dsl_wcsdup(L"ERROR in mbstowcs");
	}
	size_t llen = (len+1)*sizeof(wchar_t);
	wchar_t * buf = (wchar_t *)dsl_malloc(llen);
	memset(buf, 0, llen);
	mbstowcs(buf, str, len);
	return buf;
}

DSL_API wchar_t * DSL_CC wcstotcsW(const wchar_t * str) {
	return dsl_wcsdup(str);
}

DSL_API size_t DSL_CC wcscnt(const wchar_t * ptr) {
	size_t n = 0;
	for (; *ptr; ptr++)
		n += sizeof(wchar_t);
	return n;
}

#if !defined(HAVE_STRLCPY)
DSL_API size_t DSL_CC strlcat(char *dst, const char *src, size_t siz) {
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}

DSL_API size_t DSL_CC strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}
#endif

#if defined(MACOSX)
DSL_API wchar_t * DSL_CC wcsdup(const wchar_t *s) {
	size_t len = (wcscnt(s) + 1) * sizeof(wchar_t);
	wchar_t * copy = (wchar_t *)malloc(len);
	if (copy == NULL) { return NULL; }
	memcpy(copy, s, len);
	return copy;
}
#endif

#if defined(WIN32)
struct tm * DSL_CC localtime_r(const time_t * tme, struct tm * out) {
	if (!tme || !out) return NULL;
	if (localtime_s(out, tme) != 0) {
		memset(out, 0, sizeof(struct tm));
	}
	return out;
}
struct tm * DSL_CC gmtime_r(const time_t * tme, struct tm * out) {
	if (!tme || !out) return NULL;
	if (gmtime_s(out, tme) != 0) {
		memset(out, 0, sizeof(struct tm));
	}
	return out;
}
#endif

inline bool _file_get_contents_begin(const string& fn, int64 maxSize, int64& len, FILE ** fp) {
	*fp = fopen(fn.c_str(), "rb");
	if (*fp == NULL) {
		return false;
	}
	fseek64(*fp, 0, SEEK_END);
	len = ftell64(*fp);
	if (len > maxSize || len < 0) {
		return false;
	}
	fseek64(*fp, 0, SEEK_SET);
	return true;
}

DSL_API_CLASS size_t DSL_CC file_get_contents(const string& fn, vector<uint8>& data, int64 maxSize) {
	FILE * fp;
	int64 len;
	if (!_file_get_contents_begin(fn, maxSize, len, &fp)) {
		return false;
	}

	data.resize(len);
	bool ret = (fread(data.data(), len, 1, fp) == 1);
	fclose(fp);
	return ret;
}

DSL_API_CLASS size_t DSL_CC file_get_contents(const string& fn, string& data, int64 maxSize) {
	FILE * fp;
	int64 len;
	if (!_file_get_contents_begin(fn, maxSize, len, &fp)) {
		return false;
	}

	data.resize(len);
	bool ret = (fread(&data[0], len, 1, fp) == 1);
	fclose(fp);
	return ret;
}

DSL_API_CLASS size_t DSL_CC file_get_contents(const string& fn, uint8 ** data, int64& len, int64 maxSize) {
	FILE * fp;
	if (!_file_get_contents_begin(fn, maxSize, len, &fp)) {
		return false;
	}

	*data = (uint8 *)dsl_malloc(len);
	if (*data == NULL) {
		return false;
	}
	bool ret = (fread(*data, len, 1, fp) == 1);
	fclose(fp);
	return ret;
}

bool DSL_CC file_put_contents(const string& fn, const vector<uint8>& data, bool append) {
	FILE * fp = fopen(fn.c_str(), append ? "ab" : "wb");
	if (fp == NULL) {
		return false;
	}

	bool ret = (fwrite(data.data(), data.size(), 1, fp) == 1);
	fclose(fp);
	return ret;
}

bool DSL_CC file_put_contents(const string& fn, const string& data, bool append) {
	FILE * fp = fopen(fn.c_str(), append ? "ab" : "wb");
	if (fp == NULL) {
		return false;
	}

	bool ret = (fwrite(data.c_str(), data.length(), 1, fp) == 1);
	fclose(fp);
	return ret;
}

bool DSL_CC file_put_contents(const string& fn, const uint8 * data, size_t fileSize, bool append) {
	FILE * fp = fopen(fn.c_str(), append ? "ab" : "wb");
	if (fp == NULL) {
		return false;
	}

	bool ret = (fwrite(data, fileSize, 1, fp) == 1);
	fclose(fp);
	return ret;
}

int64 DSL_CC copy_file(const string& src, const string& dest, bool allow_overwrite) {
#ifdef WIN32
	int64 ret = filesize(src.c_str());
	if (ret >= 0 && CopyFile(src.c_str(), dest.c_str(), !allow_overwrite)) {
		return ret;
	}
	return -1;
#else
	int fdi = open(src.c_str(), O_RDONLY);
	if (fdi == -1) {
		return -1;
	}

	int flags = O_WRONLY | O_CREAT;
	if (allow_overwrite) {
		flags |= O_TRUNC;
	} else {
		flags |= O_EXCL;
	}
	int fdo = open(dest.c_str(), flags);
	if (fdo == -1) {
		close(fdi);
		return -1;
	}

	int64 ret = 0;
	#if (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 27))
	ret = copy_file_range(fdi, NULL, fdo, NULL, SIZE_MAX, 0);
	#else
	uint8 buf[32768];
	int n;
	while ((n = read(fdi, buf, sizeof(buf))) > 0) {
		if (write(fdo, buf, n) != n) {
			n = -1;
			break;
		}
		ret += n;
	}
	if (n < 0) {
		ret = -1;
	}
	#endif

	close(fdo);
	close(fdi);
	return ret;
#endif
}

/* Remaining functions past this line were borrowed from PhysicsFS */

DSL_API uint16 DSL_CC ByteSwap16(uint16 X) {
	return ( ((X >> 8)) | (X << 8) );
}

DSL_API uint32 DSL_CC ByteSwap32(uint32 X) {
    return( (X<<24) | ((X<<8) & 0x00FF0000) | ((X>>8) & 0x0000FF00) | (X>>24) );
}

DSL_API uint64 DSL_CC ByteSwap64(uint64 val) {
    uint32 lo = (uint32)(val&0xFFFFFFFF);
    val >>= 32;
    uint32 hi = (uint32)(val&0xFFFFFFFF);
    val = ByteSwap32(lo);
    val <<= 32;
    val |= ByteSwap32(hi);
    return(val);
}

#ifdef LITTLE_ENDIAN
DSL_API uint16 DSL_CC Get_ULE16(uint16 x) { return(x); }
DSL_API int16 DSL_CC Get_SLE16(int16 x) { return(x); }
DSL_API uint32 DSL_CC Get_ULE32(uint32 x) { return(x); }
DSL_API int32 DSL_CC Get_SLE32(int32 x) { return(x); }
DSL_API uint64 DSL_CC Get_ULE64(uint64 x) { return(x); }
DSL_API int64 DSL_CC Get_SLE64(int64 x) { return(x); }

DSL_API uint16 DSL_CC Get_UBE16(uint16 x) { return(ByteSwap16(x)); }
DSL_API int16 DSL_CC Get_SBE16(int16 x) { return(ByteSwap16(x)); }
DSL_API uint32 DSL_CC Get_UBE32(uint32 x) { return(ByteSwap32(x)); }
DSL_API int32 DSL_CC Get_SBE32(int32 x) { return(ByteSwap32(x)); }
DSL_API uint64 DSL_CC Get_UBE64(uint64 x) { return(ByteSwap64(x)); }
DSL_API int64 DSL_CC Get_SBE64(int64 x) { return(ByteSwap64(x)); }
#else
DSL_API uint16 DSL_CC Get_ULE16(uint16 x) { return(ByteSwap16(x)); }
DSL_API int16 DSL_CC Get_SLE16(int16 x) { return(ByteSwap16(x)); }
DSL_API uint32 DSL_CC Get_ULE32(uint32 x) { return(ByteSwap32(x)); }
DSL_API int32 DSL_CC Get_SLE32(int32 x) { return(ByteSwap32(x)); }
DSL_API uint64 DSL_CC Get_ULE64(uint64 x) { return(ByteSwap64(x)); }
DSL_API int64 DSL_CC Get_SLE64(int64 x) { return(ByteSwap64(x)); }

DSL_API uint16 DSL_CC Get_UBE16(uint16 x) { return(x); }
DSL_API int16 DSL_CC Get_SBE16(int16 x) { return(x); }
DSL_API uint32 DSL_CC Get_UBE32(uint32 x) { return(x); }
DSL_API int32 DSL_CC Get_SBE32(int32 x) { return(x); }
DSL_API uint64 DSL_CC Get_UBE64(uint64 x) { return(x); }
DSL_API int64 DSL_CC Get_SBE64(int64 x) { return(x); }
#endif

DSL_API bool DSL_CC IsValidUTF8(const char *s) {
	return (FirstInvalidUTF8(s) == NULL);
}

DSL_API const char * DSL_CC FirstInvalidUTF8(const char *p) {
	const uint8 *s = (const uint8 *)p;
	while (*s) {
		if (*s < 0x80)
			/* 0xxxxxxx */
			s++;
		else if ((s[0] & 0xe0) == 0xc0) {
			/* 110XXXXx 10xxxxxx */
			if ((s[1] & 0xc0) != 0x80 ||
				(s[0] & 0xfe) == 0xc0)                        /* overlong? */
				return (char *)s;
			else
				s += 2;
		} else if ((s[0] & 0xf0) == 0xe0) {
			/* 1110XXXX 10Xxxxxx 10xxxxxx */
			if ((s[1] & 0xc0) != 0x80 ||
				(s[2] & 0xc0) != 0x80 ||
				(s[0] == 0xe0 && (s[1] & 0xe0) == 0x80) ||    /* overlong? */
				(s[0] == 0xed && (s[1] & 0xe0) == 0xa0) ||    /* surrogate? */
				(s[0] == 0xef && s[1] == 0xbf &&
				(s[2] & 0xfe) == 0xbe))                      /* U+FFFE or U+FFFF? */
				return (char *)s;
			else
				s += 3;
		} else if ((s[0] & 0xf8) == 0xf0) {
			/* 11110XXX 10XXxxxx 10xxxxxx 10xxxxxx */
			if ((s[1] & 0xc0) != 0x80 ||
				(s[2] & 0xc0) != 0x80 ||
				(s[3] & 0xc0) != 0x80 ||
				(s[0] == 0xf0 && (s[1] & 0xf0) == 0x80) ||    /* overlong? */
				(s[0] == 0xf4 && s[1] > 0x8f) || s[0] > 0xf4) /* > U+10FFFF? */
				return (char *)s;
			else
				s += 4;
		} else
			return (char *)s;
	}

	return NULL;
}
