//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

/* MEMLEAK is deprecated, this file is just for compatibility with old code that used it */

#ifndef __DSL_MEMLEAK_H__
#define __DSL_MEMLEAK_H__

#define zmprintf dsl_mprintf
#define zvmprintf dsl_vmprintf
#define zwmprintf dsl_wmprintf

#define zmalloc(size) dsl_malloc(size)
#define zrealloc(ptr,size) dsl_realloc(ptr, size)
#define zstrdup(ptr) dsl_strdup(ptr)
#define zwcsdup(ptr) dsl_wcsdup(ptr)
#define zfree(ptr) dsl_free(ptr)
#define zfreenn(ptr) if (ptr) { dsl_free(ptr); }
#define znew(x) (x *)dsl_malloc(sizeof(x));

#define zincRef #error "zincRef only works with MEMLEAK enabled"
#define zfreeall #error "zfreeall only works with MEMLEAK enabled"

#endif // __DSL_MEMLEAK_H__
