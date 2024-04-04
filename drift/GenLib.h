//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

/**
 * \defgroup genlib General Functions Library (GenLib)
 */

/** \addtogroup genlib
 * @{
 */

#ifndef _DSL_GENLIB_H_
#define _DSL_GENLIB_H_

DSL_API_CLASS const char * DSL_CC nopathA(const char * filename); ///< Returns just the file portion of the full path and filename
DSL_API_CLASS const wchar_t * DSL_CC nopathW(const wchar_t * filename); ///< Returns just the file portion of the full path and filename
#if !defined(NO_CPLUSPLUS)
	DSL_API_CLASS char * DSL_CC nopathA(char * filename); ///< Returns just the file portion of the full path and filename
	DSL_API_CLASS wchar_t * DSL_CC nopathW(wchar_t * filename); ///< Returns just the file portion of the full path and filename
#endif

/**
 * This function takes binary data and converts it to a hex string.
 * @param data a pointer to the binary data
 * @param datalen the length of the binary data
 * @param out pointer to a char buffer to put the hex string in.
 * @param outsize The size of the buffer pointed to by out. It should have a minimum size of (datalen*2)+1
 */
DSL_API_CLASS char * DSL_CC bin2hex(const uint8_t * data, size_t datalen, char * out, size_t outsize);
/**
 * This function takes binary data and converts it to a hex string.
 * @param data a pointer to the binary data
 * @param datalen the length of the binary data
 */
DSL_API_CLASS string DSL_CC bin2hex(const uint8_t * data, size_t datalen);

/**
 * This function takes a hex string and converts it to binary data.
 * @param instr a pointer to the hex-encoded data (should be a multiple of 2)
 * @param inlen length of string instr
 * @param outsize the length of the output buffer, should be (inlen/2)
 * @param out pointer to a buffer to put the binary data in string in.
 */
DSL_API_CLASS bool DSL_CC hex2bin(const char * instr, size_t inlen, uint8 * out, size_t outsize);
/**
 * This function takes a hex string and converts it to binary data.
 * @param instr a pointer to the hex-encoded data (should be a multiple of 2). Length is determined by doing a strlen() on the string.
 * @param outsize the length of the output buffer, should be (inlen/2)
 * @param out pointer to a buffer to put the binary data in string in.
 */
DSL_API_CLASS bool DSL_CC hex2bin(const char * instr, uint8 * out, size_t outsize);
/**
 * This function takes a hex string and converts it to binary data, putting it into a vector.
 * @param instr a pointer to the hex-encoded data (should be a multiple of 2).
 */
DSL_API_CLASS bool DSL_CC hex2bin(const string instr, vector<uint8_t>& out);

DSL_API void DSL_CC PrintData(FILE * fp, const uint8 * ptr, size_t len); ///< Prints binary data in a pretty format

#if defined(WIN32) || defined(DOXYGEN_SKIP)
	DSL_API_CLASS const char * DSL_CC stristr(const char * haystack, const char * needle); ///< Case-insensitive strstr. On Linux we define a macro of stristr to strcasestr for the same effect.
	#if !defined(NO_CPLUSPLUS)
		DSL_API_CLASS char * DSL_CC stristr(char * haystack, const char * needle); ///< Case-insensitive strstr. On Linux we define a macro of stristr to strcasestr for the same effect.
	#endif
	DSL_API char * DSL_CC strtok_r(char *newstring, const char *delimiters, char ** save_ptr); ///< Windows version of strtok_r
	time_t filetime_2_time_t(FILETIME ft);
	DSL_API struct tm * DSL_CC localtime_r(const time_t * tme, struct tm * out); ///< Windows version of thread-safe localtime_r
	DSL_API struct tm * DSL_CC gmtime_r(const time_t * tme, struct tm * out); ///< Windows version of thread-safe gmtime_r
#else
	DSL_API char * DSL_CC strlwr(char * str); ///< Linux version of strlwr
	DSL_API uint32 DSL_CC GetTickCount(); ///< Linux version of GetTickCount
#endif

DSL_API bool DSL_CC dsl_mkdir_r(const char * p, int mode); ///< Cross-platform recursive mkdir()
DSL_API bool DSL_CC strempty(const char * p); ///< Returns true if a string is empty (p == NULL || *p == 0)
DSL_API int64 DSL_CC dsl_clamp(int64 value, int64 vMin, int64 vMax); ///< Clamps a given value to the range of vMin to vMax (inclusive)

DSL_API_CLASS char * DSL_CC escapeshellarg(const char * p, char * out, size_t outSize); ///< Escapes an argument for passing to shell functions, only escapes quotes atm so keep that in mind.
DSL_API_CLASS string DSL_CC escapeshellarg(const string& str); ///< Escapes an argument for passing to shell functions, only escapes quotes atm so keep that in mind.

#if defined(OPENBSD) || defined(FREEBSD)
#define HAVE_STRLCPY
#endif

#if !defined(HAVE_STRLCPY)
/**
 * Like strncpy but always null terminates and doesn't fill the whole buffer with zeros needlessly. See https://linux.die.net/man/3/strlcpy for details
 */
DSL_API size_t DSL_CC strlcpy(char * dst, const char * src, size_t siz);
/**
 * Like strncat but always null terminates and doesn't fill the whole buffer with zeros needlessly. See https://linux.die.net/man/3/strlcpy for details
 */
DSL_API size_t DSL_CC strlcat(char * dst, const char * src, size_t siz);
#endif
/**
 * Shorthand strlcpy so you don't have to use sizeof() on your char array.
 */
#define sstrcpy(x,y) strlcpy(x,y,sizeof(x))
/**
 * Shorthand strlcat so you don't have to use sizeof() on your char array.
 */
#define sstrcat(x,y) strlcat(x,y,sizeof(x))


#define TRIM_LEFT	0x01
#define TRIM_RIGHT	0x02
#define TRIM_BOTH	(TRIM_LEFT|TRIM_RIGHT)
#if !defined(NO_CPLUSPLUS) || defined(DOXYGEN_SKIP)
/**
 * Trims unwanted characters from the beginning and/or end of a string.
 * @param trim The characters to trim. Default: CR, LF, tabs, spaces.
 * @param sides The sides to trim. Default: Both
 * @sa TRIM_BOTH
 * @sa TRIM_LEFT
 * @sa TRIM_RIGHT
 */
DSL_API char * DSL_CC strtrim(char *buf, const char * trim = "\r\n\t ", uint8 sides = TRIM_BOTH);
#else
DSL_API char * DSL_CC strtrim(char *buf, const char * trim, uint8 sides);
#endif

/**
 * Case-sensitive wildcard string comparison.
 * @return 0 for no match, other for match
 */
DSL_API int DSL_CC wildcmp(const char *wild, const char *string);
/**
 * Case-insensitive wildcard string comparison.
 * @return 0 for no match, other for match
 */
DSL_API int DSL_CC wildicmp(const char *wild, const char *string);
#if !defined(NO_CPLUSPLUS) || defined(DOXYGEN_SKIP)
/**
 * Case-insensitive wildcard string comparison with multiple patterns.
 * @param sep The characters the patterns are delimited with (ala strtok). Default: |
 * @return 0 for no match, other for match
 */
DSL_API int DSL_CC wildicmp_multi(const char *wild, const char *string, const char *sep="|");
#else
DSL_API int DSL_CC wildicmp_multi(const char *wild, const char *string, const char *sep);
#endif

DSL_API int64 DSL_CC fseek64(FILE * fp, int64 offset, int whence); ///< Cross-platform 64-bit fseek.
DSL_API int64 DSL_CC ftell64(FILE * fp); ///< Cross-platform 64-bit ftell.
DSL_API int64 DSL_CC filesize(const char * fn); ///< Get the size in bytes of a file.
#if defined(WIN32) || defined(DOXYGEN_SKIP)
DSL_API int DSL_CC truncate(const char * fn, int64 size); ///< Windows version of truncate()
#endif
DSL_API_CLASS size_t DSL_CC file_get_contents(const string& fn, vector<uint8>& data, int64 maxSize=UINT32_MAX);
DSL_API_CLASS size_t DSL_CC file_get_contents(const string& fn, string& data, int64 maxSize=UINT32_MAX);
DSL_API_CLASS size_t DSL_CC file_get_contents(const string& fn, uint8 ** data, int64& fileSize, int64 maxSize=UINT32_MAX);//free data with dsl_free()

DSL_API_CLASS int DSL_CC str_replaceA(char *Str, size_t BufSize, const char *FindStr, const char *ReplStr); ///< Simple string replacement
DSL_API_CLASS int DSL_CC str_replaceW(wchar_t *Str, size_t BufSize, const wchar_t * FindStr, const wchar_t * ReplStr); ///< Simple string replacement

#if !defined(NO_CPLUSPLUS)
DSL_API_CLASS string DSL_CC str_replaceA(string str, string FindStr, string ReplStr); ///< Simple string replacement
DSL_API_CLASS wstring DSL_CC str_replaceW(wstring str, wstring FindStr, wstring ReplStr); ///< Simple string replacement
#endif

DSL_API char * DSL_CC tchar2charA(char * str);
DSL_API char * DSL_CC tchar2charW(wchar_t * str);

/**
 * Gets an appropriate directory to store config files/data for the current user. The directory will be created for you.<br>
 * Windows: %APPDATA%\name\<br>
 * Linux: /home/username/.name/
 * @return The path including trailing path separater. Free with dsl_free
 * @sa dsl_free
 */
DSL_API_CLASS char * DSL_CC GetUserConfigFolderA(const char * name);
DSL_API_CLASS string DSL_CC GetUserConfigFolderA(const string& name);
/**
 * Gets an appropriate filename to store config files/data for the current user. The directory will be created for you.<br>
 * Windows: %APPDATA%\name\fn<br>
 * Linux: /home/username/.name/fn
 * @return The path of the filename. Free with dsl_free
 * @sa dsl_free
 */
DSL_API_CLASS char * DSL_CC GetUserConfigFileA(const char * name, const char * fn);
DSL_API_CLASS string DSL_CC GetUserConfigFileA(const string& name, const string& fn);
DSL_API wchar_t * DSL_CC GetUserConfigFolderW(const wchar_t * name); ///< Unicode version of... @sa GetUserConfigFolderA
DSL_API wchar_t * DSL_CC GetUserConfigFileW(const wchar_t * name, const wchar_t * fn); ///< Unicode version of... @sa GetUserConfigFileW

/**
 * Gets an appropriate directory to store user documents for the current user. The directory will be created for you.<br>
 * Windows: %USERPROFILE%\Documents\name<br>
 * Linux: /home/username/name/
 * @return The path including trailing path separater. Free with dsl_free
 * @sa dsl_free
 */
DSL_API char * DSL_CC GetUserDocumentsFolderA(const char * name);

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
#define GetUserDocumentsFolder #error Not implemented
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
#define GetUserDocumentsFolder GetUserDocumentsFolderA
#endif


/**
 * Cross multiplication
 * Example: scale_ranges<uint16>(10, 0, 100, 0, 200) to adjust 10 from a range of 0-100 to a range of 0-200 which would be 20.
 */
template <typename T>
T scale_ranges(T value, T srcmin, T srcmax, T destmin, T destmax) {
	T in = value - srcmin;
	T indiff = srcmax - srcmin;
	T outdiff = destmax - destmin;
	return ((in * outdiff) / indiff) + destmin;
}

#if !defined(NO_CPLUSPLUS) || defined(DOXYGEN_SKIP)
/**
 * Clamps a given value to the range of vMin to vMax (inclusive)<br>
 * Example: clamp<uint16>(10, 0, 15) to clamp a value of 10 to a range of 0-15.
 */
#if defined(__cplusplus) && __cplusplus < 201703L
template <typename T>
T clamp(T v, T vMin, T vMax) {
	if (v < vMin) { return vMin; }
	if (v > vMax) { return vMax; }
	return v;
}
#endif

/**
 * String tokenizer class, similar to strtok but only using one character as a delimiter. This was inspired by mIRC back in the day so the tokens are base 1 instead of base 0 for their indexes.
 */
class DSL_API_CLASS StrTokenizer {
private:
	bool b_strdup = false;
	char * string = NULL;
	char sep = 0;
	char sep_str[2] = { 0,0 };

	size_t num_tokens = 0;
	char ** tokens = NULL;
public:
	/**
	 * @param str The string to split up
	 * @param separater The character to split at
	 * @param do_strdup If true we will strdup the string and operate on our own copy. If false the original string you pass will be modified.
	 */
	StrTokenizer(char * str, char separater, bool do_strdup=true);
	~StrTokenizer();

	size_t NumTok();

	char * GetTok(size_t first, size_t last);
	char * GetSingleTok(size_t num);
	void FreeString(char * buf); ///< You must call this on any string returned by GetTok/GetSingleTok

	std::string stdGetTok(size_t first, size_t last);
	std::string stdGetSingleTok(size_t num);
};
#endif

/* Remaining functions past this line were borrowed from PhysicsFS */

DSL_API bool DSL_CC IsValidUTF8(const char *s); ///< Checks if the string is valid UTF-8 (no invalid UTF-8 sequences, etc.)
DSL_API const char * DSL_CC FirstInvalidUTF8(const char *s);

/**@}*/

/**
 * \defgroup byteswap Byte Swapping
 * We also define LITTLE_ENDIAN or BIG_ENDIAN for you as well.
 */

/** \addtogroup byteswap
 * @{
 */


DSL_API uint16 DSL_CC ByteSwap16(uint16 X); ///< Byte swap a 16-bit unsigned integer
DSL_API uint32 DSL_CC ByteSwap32(uint32 X); ///< Byte swap a 32-bit unsigned integer
DSL_API uint64 DSL_CC ByteSwap64(uint64 val); ///< Byte swap a 64-bit unsigned integer

DSL_API uint16 DSL_CC Get_ULE16(uint16 x); ///< Convert from the native endianness to Little Endian
DSL_API int16 DSL_CC Get_SLE16(int16 x); ///< Convert from the native endianness to Little Endian
DSL_API uint32 DSL_CC Get_ULE32(uint32 x); ///< Convert from the native endianness to Little Endian
DSL_API int32 DSL_CC Get_SLE32(int32 x); ///< Convert from the native endianness to Little Endian
DSL_API uint64 DSL_CC Get_ULE64(uint64 x); ///< Convert from the native endianness to Little Endian
DSL_API int64 DSL_CC Get_SLE64(int64 x); ///< Convert from the native endianness to Little Endian

DSL_API uint16 DSL_CC Get_UBE16(uint16 x); ///< Convert from the native endianness to Big Endian
DSL_API int16 DSL_CC Get_SBE16(int16 x); ///< Convert from the native endianness to Big Endian
DSL_API uint32 DSL_CC Get_UBE32(uint32 x); ///< Convert from the native endianness to Big Endian
DSL_API int32 DSL_CC Get_SBE32(int32 x); ///< Convert from the native endianness to Big Endian
DSL_API uint64 DSL_CC Get_UBE64(uint64 x); ///< Convert from the native endianness to Big Endian
DSL_API int64 DSL_CC Get_SBE64(int64 x); ///< Convert from the native endianness to Big Endian

/**@}*/

#endif // _DSL_GENLIB_H_
