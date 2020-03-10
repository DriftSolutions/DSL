//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef _GENLIB_H_
#define _GENLIB_H_

DSL_API_CLASS const char * DSL_CC nopathA(const char * filename);
DSL_API_CLASS const wchar_t * DSL_CC nopathW(const wchar_t * filename);
#if !defined(NO_CPLUSPLUS)
	DSL_API_CLASS char * DSL_CC nopathA(char * filename);
	DSL_API_CLASS wchar_t * DSL_CC nopathW(wchar_t * filename);
#endif

/*
	This function takes binary data and converts it to a hex string.
	@param data a pointer to the binary data
	@param datalen the length of the binary data
	@param out pointer to a char buffer to put the hex string in. out should have a minimum size of (datalen*2)+1
*/
DSL_API_CLASS char * DSL_CC bin2hex(const uint8_t * in, size_t inlen, char * out, size_t outsize);
DSL_API_CLASS string DSL_CC bin2hex(const uint8_t * in, size_t inlen);

/*
This function takes binary data and converts it to a hex string.
@param instr a pointer to the hex-encoded data (should be a multiple of 2)
@param inline length of string instr (in the version without inlen strlen() is run on instr)
@param outsize the length of the output buffer, should be (len(in)/2) + 1
@param out pointer to a buffer to put the binary data in string in.
*/
DSL_API_CLASS bool DSL_CC hex2bin(const char * instr, uint8 * out, size_t outsize);
DSL_API_CLASS bool DSL_CC hex2bin(const char * instr, size_t inlen, uint8 * out, size_t outsize);
DSL_API_CLASS bool DSL_CC hex2bin(const string instr, vector<uint8_t>& out);

/* Prints binary data in a pretty format */
DSL_API void DSL_CC PrintData(FILE * fp, const uint8 * ptr, size_t len);

#if defined(WIN32)
	DSL_API_CLASS const char * DSL_CC stristr(const char * haystack, const char * needle);
	#if !defined(NO_CPLUSPLUS)
		DSL_API_CLASS char * DSL_CC stristr(char * haystack, const char * needle);
	#endif
	DSL_API char * DSL_CC strtok_r (char *newstring, const char *delimiters, char ** save_ptr);
	time_t filetime_2_time_t(FILETIME ft);
	DSL_API struct tm * DSL_CC localtime_r(const time_t * tme, struct tm * out);
	DSL_API struct tm * DSL_CC gmtime_r(const time_t * tme, struct tm * out);
#else
	DSL_API char * DSL_CC strlwr(char * str);
	DSL_API uint32 DSL_CC GetTickCount();
#endif

DSL_API bool DSL_CC dsl_mkdir_r(const char * p, int mode);
DSL_API bool DSL_CC strempty(const char * p);
DSL_API int64 DSL_CC dsl_clamp(int64 value, int64 vMin, int64 vMax);

DSL_API char * DSL_CC escapeshellarg(const char * p, char * out, size_t outSize);

#if defined(OPENBSD) || defined(FREEBSD)
#define HAVE_STRLCPY
#endif

#if !defined(HAVE_STRLCPY)
/*
 * Like strncpy/strncat but always null terminates and doesn't fill the whole buffer with zeros needlessly. See https://linux.die.net/man/3/strlcpy for details
 */
DSL_API size_t DSL_CC strlcpy(char * dst, const char * src, size_t siz);
DSL_API size_t DSL_CC strlcat(char * dst, const char * src, size_t siz);
#endif

#define TRIM_LEFT	0x01
#define TRIM_RIGHT	0x02
#define TRIM_BOTH	(TRIM_LEFT|TRIM_RIGHT)
#if !defined(NO_CPLUSPLUS)
DSL_API char * DSL_CC strtrim(char *buf, const char * trim = "\r\n\t ", uint8 sides = TRIM_BOTH);
#else
DSL_API char * DSL_CC strtrim(char *buf, const char * trim, uint8 sides);
#endif

/* wildcmp functions return 0 for no match, other for match */
DSL_API int DSL_CC wildcmp(const char *wild, const char *string);
DSL_API int DSL_CC wildicmp(const char *wild, const char *string);
#if !defined(NO_CPLUSPLUS)
DSL_API int DSL_CC wildicmp_multi(const char *wild, const char *string, const char *sep="|");
#else
DSL_API int DSL_CC wildicmp_multi(const char *wild, const char *string, const char *sep);
#endif

DSL_API int64 DSL_CC fseek64(FILE * fp, int64 offset, int whence);
DSL_API int64 DSL_CC ftell64(FILE * fp);
DSL_API int64 DSL_CC filesize(const char * fn);
#ifdef WIN32
DSL_API int DSL_CC truncate(const char * fn, int64 size);
#endif

DSL_API int DSL_CC str_replaceA(char *Str, unsigned long BufSize, const char *FindStr, const char *ReplStr);
DSL_API int DSL_CC str_replaceW(wchar_t *Str, unsigned long BufSize, const wchar_t * FindStr, const wchar_t * ReplStr);

DSL_API char * DSL_CC tchar2charA(char * str);
DSL_API char * DSL_CC tchar2charW(wchar_t * str);

DSL_API char * DSL_CC GetUserConfigFolderA(const char * name);
DSL_API char * DSL_CC GetUserConfigFileA(const char * name, const char * fn);
DSL_API wchar_t * DSL_CC GetUserConfigFolderW(const wchar_t * name);
DSL_API wchar_t * DSL_CC GetUserConfigFileW(const wchar_t * name, const wchar_t * fn);

DSL_API char * DSL_CC tcstombsA(const char * str);
DSL_API char * DSL_CC tcstombsW(const wchar_t * str);
DSL_API wchar_t * DSL_CC tcstowcsW(const wchar_t * str);
DSL_API wchar_t * DSL_CC tcstowcsA(const char * str);
DSL_API char * DSL_CC mbstotcsA(const char * str);
DSL_API char * DSL_CC wcstotcsA(const wchar_t * str);
DSL_API wchar_t * DSL_CC mbstotcsW(const char * str);
DSL_API wchar_t * DSL_CC wcstotcsW(const wchar_t * str);
DSL_API size_t DSL_CC wcscnt(const wchar_t * ptr); // returns number of bytes taken up by string, not including NULL terminator


#ifdef UNICODE
#define str_replace str_replaceW
#define tchar2char tchar2charW
#define tcstombs tcstombsW
#define tcstowcs tcstowcsW
#define mbstotcs mbstotcsW
#define wcstotcs wcstotcsW
#define nopath nopathW
#define GetUserConfigFolder GetUserConfigFolderW
#define GetUserConfigFile GetUserConfigFileW
#else
#define str_replace str_replaceA
#define tchar2char tchar2charA
#define tcstombs tcstombsA
#define tcstowcs tcstowcsA
#define mbstotcs mbstotcsA
#define wcstotcs wcstotcsA
#define nopath nopathA
#define GetUserConfigFolder GetUserConfigFolderA
#define GetUserConfigFile GetUserConfigFileA
#endif

#if !defined(NO_CPLUSPLUS)
template <typename T>
T clamp(T v, T vMin, T vMax) {
	if (v < vMin) { return vMin; }
	if (v > vMax) { return vMax; }
	return v;
}

class DSL_API_CLASS StrTokenizer {
private:
	bool b_strdup;
	char * string;
	int sep;
	char sep_str[2];

	unsigned int num_tokens;
	char ** tokens;
public:
	StrTokenizer(char * str, int separater, bool do_strdup=true);
	~StrTokenizer();

	unsigned int NumTok();

	char * GetTok(unsigned int first, unsigned int last);
	char * GetSingleTok(unsigned int num);
	void FreeString(char * buf); ///< You must call this on any string returned by GetTok/GetSingleTok

	std::string stdGetTok(unsigned int first, unsigned int last);
	std::string stdGetSingleTok(unsigned int num);
};
#endif

/* Remaining functions past this line were borrowed from PhysicsFS */

DSL_API bool DSL_CC IsValidUTF8(const char *s);
DSL_API const char * DSL_CC FirstInvalidUTF8(const char *s);

DSL_API uint16 DSL_CC ByteSwap16(uint16 X);
DSL_API uint32 DSL_CC ByteSwap32(uint32 X);
DSL_API uint64 DSL_CC ByteSwap64(uint64 val);

DSL_API uint32 DSL_CC Get_ULE16(uint16 x);
DSL_API int32 DSL_CC Get_SLE16(int16 x);
DSL_API uint32 DSL_CC Get_ULE32(uint32 x);
DSL_API int32 DSL_CC Get_SLE32(int32 x);
DSL_API uint64 DSL_CC Get_ULE64(uint64 x);
DSL_API int64 DSL_CC Get_SLE64(int64 x);

DSL_API uint16 DSL_CC Get_UBE16(uint16 x);
DSL_API int16 DSL_CC Get_SBE16(int16 x);
DSL_API uint32 DSL_CC Get_UBE32(uint32 x);
DSL_API int32 DSL_CC Get_SBE32(int32 x);
DSL_API uint64 DSL_CC Get_UBE64(uint64 x);
DSL_API int64 DSL_CC Get_SBE64(int64 x);

#endif // _GENLIB_H_
