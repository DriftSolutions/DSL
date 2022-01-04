//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dslcore.h>
#include <drift/Directory.h>
#include <drift/GenLib.h>

Directory::Directory() {
	hFind = NULL;
	mDir = NULL;
#if defined(__DSL_USE_READDIR_R__)
	last_res = NULL;
#endif
}

Directory::Directory(const char * dir) {
	hFind = NULL;
	mDir = NULL;
#if defined(__DSL_USE_READDIR_R__)
	last_res = NULL;
#endif
	Open(dir);
}

Directory::Directory(const wchar_t * dir) {
	hFind = NULL;
	mDir = NULL;
#if defined(__DSL_USE_READDIR_R__)
	last_res = NULL;
#endif
	Open(dir);
}

Directory::~Directory() {
	Close();
}

bool Directory::Open(const char * dir) {
	this->Close();
	if (dir[0] == 0) { return false; }
	this->mDir = new char[strlen(dir)+16];
	strcpy(mDir, dir);
#if defined(WIN32)
//	if (mDir[strlen(mDir)-1] != PATH_SEP) { strcat(mDir, PATH_SEPS); }
//	strcat(mDir,"*");

	int bufSize = int(strlen(dir)) + 16;
	wchar_t * wbuf = new wchar_t[bufSize];
	int n = MultiByteToWideChar(CP_UTF8, 0, dir, -1, wbuf, bufSize);
	if (n == 0) {
		n = MultiByteToWideChar(CP_THREAD_ACP, 0, dir, -1, wbuf, bufSize);
	}
	if (n == 0) {
		return false;
	}
	bool ret = Open(wbuf);
	delete [] wbuf;
	return ret;
#else
	if (mDir[strlen(mDir)-1] == PATH_SEP) { mDir[strlen(mDir)-1]=0; }
#if defined(__DSL_USE_READDIR_R__)
	last_res = NULL;
#endif
	hFind = opendir(mDir);
	strcat(mDir, PATH_SEPS);
	if (!this->hFind) {
		return false;
	}
	return true;
#endif
}

bool Directory::Open(const wchar_t * dir) {
	this->Close();
	if (wcslen(dir) == 0) { return false; }
	this->wDir = new wchar_t[wcslen(dir)+16];
	wcscpy(wDir, dir);
#if defined(WIN32)
	if (wDir[wcslen(wDir)-1] != PATH_SEP) { wcscat(wDir, WPATH_SEPS); }
	wcscat(wDir, L"*");
#else
	//we do not yet support Unicode on Linux (don't know which APIs to use)
	char dir2[MAX_PATH];
	wcstombs(dir2, dir, sizeof(dir2));
	return Open(dir2);
#endif
	return true;
}

bool Directory::Read(char * buf, unsigned long bufSize, bool * is_dir, int64 * size) {
	memset(buf, 0, bufSize);
#if defined(WIN32)
	wchar_t * wbuf = new wchar_t[bufSize+1];
	bool ret = Read(wbuf, bufSize, is_dir, size);
	WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, bufSize, NULL, NULL);
	delete [] wbuf;
	return ret;
	/*
	if (!this->hFind) {
		this->hFind = FindFirstFileA(mDir, &wfdA);
		strrchr(mDir, PATH_SEP)[1]=0;
		if (hFind != INVALID_HANDLE_VALUE) {
			if (is_dir) {
				*is_dir = (wfdA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true:false;
			}
			if (size) {
				LARGE_INTEGER li;
				li.LowPart = wfdA.nFileSizeLow;
				li.HighPart = wfdA.nFileSizeHigh;
				*size = li.QuadPart;
			}
			strlcpy(buf, wfdA.cFileName, bufSize);
			return true;
		} else {
			hFind = NULL;
		}
	} else {
		if (FindNextFileA(this->hFind, &wfdA)) {
			if (is_dir) {
				*is_dir = (wfdA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true:false;
			}
			if (size) {
				LARGE_INTEGER li;
				li.LowPart = wfdA.nFileSizeLow;
				li.HighPart = wfdA.nFileSizeHigh;
				*size = li.QuadPart;
			}
			strlcpy(buf, wfdA.cFileName, bufSize);
			return true;
		}
	}
	*/
#else
	if (!this->hFind) {
		return false;
	}

#if !defined(__DSL_USE_READDIR_R__)
	dirent * de = readdir(hFind);
	if (de != NULL) {
#else
	dirent rde;
	dirent * de = &rde;
	if (readdir_r(hFind, &de, &last_res) == 0 && last_res != NULL) {
#endif
		/* If the d_type is DT_LNK then we'll have to stat it to know if it's a directory or not */
		if (is_dir != NULL && size == NULL && (de->d_type == DT_DIR || de->d_type == DT_REG)) {
			*is_dir = (de->d_type == DT_DIR) ? true:false;
		} else if (is_dir != NULL || size != NULL) {
			snprintf(buf, bufSize,"%s%s", mDir, de->d_name);
			struct stat st;
			if (stat(buf, &st) == 0) {
				if (is_dir) {
					*is_dir = S_ISDIR(st.st_mode) ? true:false;
				}
				if (size) {
					*size = st.st_size;
				}
			}
		}
		strlcpy(buf, de->d_name, bufSize);
		return true;
	}

#endif
	return false;
}

bool Directory::Read(wchar_t * buf, unsigned long bufSize, bool * is_dir, int64 * size) {
	memset(buf, 0, bufSize);
#if defined(WIN32)
	if (hFind == NULL) {
		hFind = FindFirstFileW(wDir, &wfdW);
		wcsrchr(wDir, PATH_SEP)[1] = 0;
		if (hFind != INVALID_HANDLE_VALUE) {
			if (is_dir) {
				*is_dir = (wfdW.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
			}
			if (size) {
				LARGE_INTEGER li;
				li.LowPart = wfdW.nFileSizeLow;
				li.HighPart = wfdW.nFileSizeHigh;
				*size = li.QuadPart;
			}
			wcsncpy(buf, wfdW.cFileName, bufSize);
			return true;
		} else {
			hFind = NULL;
		}
	} else {
		if (FindNextFileW(this->hFind, &wfdW)) {
			if (is_dir) {
				*is_dir = (wfdW.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
			}
			if (size) {
				LARGE_INTEGER li;
				li.LowPart = wfdW.nFileSizeLow;
				li.HighPart = wfdW.nFileSizeHigh;
				*size = li.QuadPart;
			}
			wcsncpy(buf, wfdW.cFileName, bufSize);
			return true;
		}
	}
#else
	char * tmp = (char *)dsl_malloc(bufSize+1);
	tmp[bufSize] = 0;
	bool ret = Read(tmp, bufSize, is_dir, size);
	if (ret) {
		mbstowcs(buf, tmp, bufSize);
	}
	dsl_free(tmp);
	return ret;
#endif
	return false;
}

void Directory::Close() {
	if (hFind) {
#if defined(WIN32)
		FindClose(hFind);
#else
		closedir(hFind);
#endif
		hFind = NULL;
	}
	if (this->mDir) {
		delete [] mDir;
		mDir = NULL;
	}
}
