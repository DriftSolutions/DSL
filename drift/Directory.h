//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef __GLIBC__
	#if __GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 24)
		#warning "Old glibc, using readdir_r"
		#define __DSL_USE_READDIR_R__ 1
	#endif
#endif

class DSL_API_CLASS Directory {
private:
	union {
		char * mDir;
		wchar_t * wDir;
	};
	#if defined(WIN32)
		HANDLE hFind;
		union {
			WIN32_FIND_DATAA wfdA;
			WIN32_FIND_DATAW wfdW;
		};
	#else
		DIR * hFind;
		#if defined(__DSL_USE_READDIR_R__)
			dirent * last_res;
		#endif
	#endif

public:
	Directory();
	Directory(const char * dir);
	Directory(const wchar_t * dir);
	~Directory();

	bool Open(const wchar_t * dir);
	bool Open(const char * dir);

	bool Read(wchar_t * buf, unsigned long bufSize, bool * is_dir=NULL, int64 * size=NULL);
	bool Read(char * buf, unsigned long bufSize, bool * is_dir=NULL, int64 * size=NULL);

	void Close();
};
