//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef __INI_READER_H__
#define __INI_READER_H__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__cplusplus) || defined(NO_CPLUSPLUS)
DSL_API char * DSL_CC Get_INI_String(const char * pszFilename, const char *pszSection, const char *pszItem, char *pszValue, size_t lValueBufferSize, const char * pszDefault);
DSL_API char * DSL_CC Get_INI_String_Memory(const char * cINIFile, const char *pszSection, const char *pszItem, char *pszValue, size_t lValueBufferSize, const char * pszDefault);
DSL_API int64 DSL_CC Get_INI_Int(const char *path, const char *section, const char *item, int64 iDefault);
#else
DSL_API char * DSL_CC Get_INI_String(const char * pszFilename, const char *pszSection, const char *pszItem, char *pszValue, size_t lValueBufferSize, const char * pszDefault = NULL);
DSL_API int64 DSL_CC Get_INI_Int(const char *path, const char *section, const char *item, int64 iDefault = 0);

DSL_API char * DSL_CC Get_INI_String_Memory(const char * cINIFile, const char *pszSection, const char *pszItem, char *pszValue, size_t lValueBufferSize, const char * pszDefault = NULL);
DSL_API int64 DSL_CC Get_INI_Int_Memory(const char * cINIFile, const char *section, const char *item, int64 iDefault = 0);
#endif

DSL_API int DSL_CC Write_INI_String(const char *pszPath, const char *pszSection, const char *pszItem, const char *pszValue);
DSL_API int DSL_CC Write_INI_Int(const char *pszPath, const char *pszSection, const char *pszItem, int64 iValue);

#ifdef __cplusplus
}
#endif

#endif // __INI_READER_H__

