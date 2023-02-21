//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __INI_READER_H__
#define __INI_READER_H__

/**
 * \defgroup ini INI File Reader/Writer
 */

/** \addtogroup ini
 * @{
 */

DSL_API char * DSL_CC Get_INI_String(const char * pszFilename, const char *pszSection, const char *pszItem, char *pszValue, size_t lValueBufferSize, const char * pszDefault = NULL);
DSL_API int64 DSL_CC Get_INI_Int(const char *pszFilename, const char *section, const char *item, int64 iDefault = 0);
DSL_API double DSL_CC Get_INI_Float(const char * pszFilename, const char *section, const char *item, double iDefault = 0);

DSL_API char * DSL_CC Get_INI_String_Memory(const char * cINIFile, const char *pszSection, const char *pszItem, char *pszValue, size_t lValueBufferSize, const char * pszDefault = NULL);
DSL_API int64 DSL_CC Get_INI_Int_Memory(const char * cINIFile, const char *section, const char *item, int64 iDefault = 0);
DSL_API double DSL_CC Get_INI_Float_Memory(const char * cINIFile, const char *section, const char *item, double iDefault = 0);

DSL_API int DSL_CC Write_INI_String(const char *pszPath, const char *pszSection, const char *pszItem, const char *pszValue);
DSL_API int DSL_CC Write_INI_Int(const char *pszPath, const char *pszSection, const char *pszItem, int64 iValue);

/**@}*/

#endif // __INI_READER_H__

