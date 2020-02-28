//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#define WHEREIS_MAX_RESULTS 256
typedef struct {
	int nCount;
	char * sResults[WHEREIS_MAX_RESULTS];
} WHEREIS_RESULTS;

DSL_API WHEREIS_RESULTS * DSL_CC WhereIs(const char * fn);
DSL_API void DSL_CC WhereIs_FreeResults(WHEREIS_RESULTS * ptr);
