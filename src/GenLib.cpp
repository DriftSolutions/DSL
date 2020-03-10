//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
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

StrTokenizer::StrTokenizer(char * str, int separater, bool do_strdup) {
	b_strdup = do_strdup;
	if (b_strdup) {
	 	string = dsl_strdup(str);
	} else {
		string = str;
	}
	sep = separater;
	sprintf(sep_str,"%c",sep);

	bool ends_in_sep = false;
	if (string[strlen(string)-1] == sep) {
		ends_in_sep = true;
	}

	num_tokens=0;
	tokens = NULL;
	char * p2 = NULL;
	char * p = strtok_r(string,sep_str,&p2);
	while (p) {
		num_tokens++;
		tokens = (char **)dsl_realloc(tokens, sizeof(char *)*num_tokens);
		tokens[num_tokens-1] = p;
		p = strtok_r(NULL,sep_str,&p2);
	}
	if (ends_in_sep) {
		num_tokens++;
		tokens = (char **)dsl_realloc(tokens, sizeof(char *)*num_tokens);
		tokens[num_tokens-1] = p2;
	}

	//printf("numtok: %u\n", num_tokens);
}

StrTokenizer::~StrTokenizer() {
	if (b_strdup) {
		dsl_free(string);
	}
	dsl_freenn(tokens);
}

unsigned int StrTokenizer::NumTok() {
	return num_tokens;
	/*
	unsigned int ret=0;

	char * tmp = dsl_strdup(string);
	char * p2 = NULL;
	char * p = strtok_r(tmp,sep_str,&p2);
	while (p) {
		ret++;
		p = strtok_r(NULL,sep_str,&p2);
	}
	dsl_free(tmp);
	if (string[strlen(string)-1] == sep) {
		ret++;
	}

	//printf("numtok: %u\n",ret);
	return ret;
	*/
}

char * StrTokenizer::GetTok(unsigned int first, unsigned int last) {

	size_t lSize=1;
	char * ret = (char *)dsl_malloc(lSize);
	ret[0]=0;

	if (first < 1) { first = 1; }
	if (last > num_tokens) { last = num_tokens; }
	for (unsigned int i=(first-1); i < last; i++) {
		if (lSize > 1) {
			lSize += strlen(tokens[i]) + 1; // string + separater
			ret = (char *)dsl_realloc(ret,lSize);
			strcat(ret,sep_str);
			strcat(ret,tokens[i]);
		} else {
			lSize += strlen(tokens[i]);
			ret = (char *)dsl_realloc(ret,lSize);
			strcpy(ret,tokens[i]);
		}
	}

	return ret;
}

char * StrTokenizer::GetSingleTok(unsigned int num) {
	return GetTok(num,num);
};

void StrTokenizer::FreeString(char * buf) {
	dsl_free(buf);
}

std::string StrTokenizer::stdGetTok(unsigned int first, unsigned int last) {
	//size_t num=0;
	std::string ret="";

	if (first < 1) { first = 1; }
	if (last > num_tokens) { last = num_tokens; }
	for (unsigned int i=(first-1); i < last; i++) {
		if (ret.length()) {
			ret += sep_str;
			ret += tokens[i];
		} else {
			ret = tokens[i];
		}
	}

	return ret;
}

std::string StrTokenizer::stdGetSingleTok(unsigned int num) {
	return stdGetTok(num,num);
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
		unsigned int tostep = ((len - beg) >= 16) ? 16 : (len - beg);

		fprintf(fp, "%08xh: ", beg);

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
	int32 i=0;
	int32 len = strlen(trim);
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
	dsl_mkdir(newdir, mode);

	dsl_free(tmp);
	dsl_free(newdir);
	return (access(newdir, F_OK) == 0);
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

int DSL_CC str_replaceA(char *Str, unsigned long BufSize, const char *FindStr, const char *ReplStr) {
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
int DSL_CC str_replaceW(wchar_t * Str, unsigned long BufSize, wchar_t *FindStr, wchar_t *ReplStr) {
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
		snwprintf(fmt, 32, L"%%.%ds%%s%%s", len);
		snwprintf(Str, BufSize, fmt, Begin, ReplStr, End);
		dsl_free(End);
		dsl_free(Begin);
		ret++;
	}
	return ret;
}
#endif

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
	if (buf[strlen(buf)-1] != PATH_SEP) {
		strlcat(buf, PATH_SEPS, sizeof(buf));
	}
#if !defined(WIN32)
	strlcat(buf, ".", sizeof(buf));
#endif
	strlcat(buf, name, sizeof(buf));
	struct stat st;
	if (stat(buf, &st) != 0) {
#ifdef WIN32
		mkdir(buf);
#else
		mkdir(buf, 0755);
#endif
	}
	strcat(buf, PATH_SEPS);
	return dsl_strdup(buf);
}

char * DSL_CC GetUserConfigFileA(const char * name, const char * fn) {
	char * dir = GetUserConfigFolderA(name);
	char * ret = dsl_mprintf("%s%s", dir, fn);
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
