//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#include <drift/dslcore.h>
#include <drift/WhereIs.h>
#include <drift/Directory.h>
#include <drift/GenLib.h>

void WhereIs_Search(WHEREIS_RESULTS * ret, const char * sDir, const char * fn) {
	char buf[MAX_PATH],buf2[MAX_PATH];
	memset(&buf, 0, sizeof(buf));
	memset(&buf2, 0, sizeof(buf2));
	bool is_dir = false;
	Directory dir(sDir);
	while (dir.Read(buf, sizeof(buf), &is_dir)) {
		if (!is_dir) {
			if (wildicmp(fn, buf)) {
				sprintf(buf2, "%s%s", sDir, buf);
				ret->sResults[ret->nCount] = dsl_strdup(buf2);
				ret->nCount++;
			}
		}
	}
}

WHEREIS_RESULTS * DSL_CC WhereIs(const char * fn) {
	WHEREIS_RESULTS * ret = (WHEREIS_RESULTS *)dsl_malloc(sizeof(WHEREIS_RESULTS));
	memset(ret, 0, sizeof(WHEREIS_RESULTS));
	char dir[MAX_PATH];
	memset(&dir, 0, sizeof(dir));
	if (getcwd(dir, sizeof(dir))) {
		if (dir[strlen(dir) - 1] != PATH_SEP) {
			strcat(dir, PATH_SEPS);
		}
		WhereIs_Search(ret, dir, fn);
	}
#ifdef WIN32
	if (SHGetSpecialFolderPathA(NULL, dir, CSIDL_SYSTEM, FALSE)) {
		if (dir[strlen(dir) - 1] != PATH_SEP) {
			strcat(dir, PATH_SEPS);
		}
		WhereIs_Search(ret, dir, fn);
	}
	if (SHGetSpecialFolderPathA(NULL, dir, CSIDL_WINDOWS, FALSE)) {
		if (dir[strlen(dir) - 1] != PATH_SEP) {
			strcat(dir, PATH_SEPS);
		}
		WhereIs_Search(ret, dir, fn);
	}
#else
	#if defined(__x86_64__)
	WhereIs_Search(ret, "/usr/lib64/", fn);
	WhereIs_Search(ret, "/usr/local/lib64/", fn);
	WhereIs_Search(ret, "/lib64/", fn);
	WhereIs_Search(ret, "/usr/lib/x86_64-linux-gnu/", fn);
	#endif
	WhereIs_Search(ret, "/usr/lib/", fn);
	WhereIs_Search(ret, "/usr/local/lib/", fn);
	WhereIs_Search(ret, "/lib/", fn);
	WhereIs_Search(ret, "/usr/lib/i386-linux-gnu/", fn);

	const char * ld = getenv("LD_LIBRARY_PATH");
	if (ld && strlen(ld)) {
		char * tmp = dsl_strdup(ld);
		char * p2 = NULL;
		char * p = strtok_r(tmp, ":", &p2);
		while (p) {
			strcpy(dir, p);
			if (dir[strlen(dir) - 1] != PATH_SEP) {
				strcat(dir, PATH_SEPS);
			}
			WhereIs_Search(ret, dir, fn);
			p = strtok_r(NULL, ":", &p2);
		}
		dsl_free(tmp);
	}
#endif
	return ret;
}

void DSL_CC WhereIs_FreeResults(WHEREIS_RESULTS * ptr) {
	if (ptr == NULL) { return; }

	for (int i=0; i < ptr->nCount; i++) {
		dsl_free(ptr->sResults[i]);
	}
	dsl_free(ptr);
}
