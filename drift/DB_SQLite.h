//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef _INCLUDE_SQLITECONX_H_
#define _INCLUDE_SQLITECONX_H_

#include <drift/DB_Common.h>
#include <sqlite3.h>

#if defined(DSL_DLL) && defined(WIN32)
	#if defined(DSL_SQLITE_EXPORTS)
		#define DSL_SQLITE_API extern "C" __declspec(dllexport)
		#define DSL_SQLITE_API_CLASS __declspec(dllexport)
	#else
		#define DSL_SQLITE_API extern "C" __declspec(dllimport)
		#define DSL_SQLITE_API_CLASS __declspec(dllimport)
	#endif
#else
	#define DSL_SQLITE_API DSL_API_VIS
	#define DSL_SQLITE_API_CLASS DSL_API_VIS
#endif

/**
 * \defgroup sqlite SQLite Database Wrapper
 */

/** \addtogroup sqlite
 * @{
 */

typedef int (*DB_SQLite_Query_Callback)(void * user_ptr, int ncols, char ** values, char ** cols);
typedef bool (*DB_SQLite_Query_Row_Callback)(const SC_Row& row, void * user_ptr);

class DSL_SQLITE_API_CLASS SQLite_Result {
public:
	size_t ind = 0;
	vector<SC_Row> rows;
};

class DSL_SQLITE_API_CLASS DB_SQLite: public SQLConx {
public:
	DB_SQLite();
	~DB_SQLite();

	bool Open(const string& filename);
	bool OpenV2(const string& filename, int flags, const string& vfs = "");
	bool IsOpen();
	void Close();

	string GetErrorString();
	int GetError();

	int Query(const string& query, DB_SQLite_Query_Callback cb, void * user_ptr = NULL);
	bool Query(const string& query, DB_SQLite_Query_Row_Callback cb, void * user_ptr = NULL);
	SQLite_Result * Query(const string& query); // this is only recommended for small result sets since it will load the entire result into memory
	size_t NumRows(SQLite_Result *result);
	bool FetchRow(SQLite_Result *result, SC_Row& retRow); // Copies the current row in to retRow, advances the internal index to the next row
	bool FetchRow(SQLite_Result *result, SC_Row ** retRow); // Stores a pointer to the current row in to retRow, advances the internal index to the next row
	bool FreeResult(SQLite_Result *result);

	bool NoResultQuery(const string& query, bool silent_errors = false);
	virtual bool Insert(const string& table, const SC_Row& row);
	virtual bool InsertIgnore(const string& table, const SC_Row& row);
	virtual bool InsertOrUpdate(const string& table, const SC_Row& row);
	virtual bool Replace(const string& table, const SC_Row& row);

	uint32 InsertID();
	int64 InsertID64();
	int AffectedRows();
#if SQLITE_VERSION_NUMBER >= 3037000
	int64 AffectedRows64();
#endif
	uint32 GetQueryCount();

	string EscapeString(const string& str);
	string MPrintf(const char * str, ...); // sqlite3 extensions: %q escape, %Q escape and put quotes around entire thing
	sqlite3 * GetHandle();

	SQLConxMulti * MultiStart();
	bool MultiSend(SQLConxMulti *);
	bool MultiEnd(SQLConxMulti *);

private:
	sqlite3 * handle = NULL;
	uint32_t query_count = 0;
	bool _insert(const string& table, const SC_Row& row, const string& action);
};

/**@}*/

#endif //_INCLUDE_SQLITECONX_H_
