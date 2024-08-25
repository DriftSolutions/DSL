//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef _INCLUDE_SQLCONX_H_
#define _INCLUDE_SQLCONX_H_

#include <drift/DB_Common.h>
#include <mysql/mysql.h>

#if defined(DSL_DLL) && defined(WIN32)
	#if defined(DSL_MYSQL_EXPORTS)
		#define DSL_MYSQL_API extern "C" __declspec(dllexport)
		#define DSL_MYSQL_API_CLASS __declspec(dllexport)
	#else
		#define DSL_MYSQL_API extern "C" __declspec(dllimport)
		#define DSL_MYSQL_API_CLASS __declspec(dllimport)
	#endif
#else
	#define DSL_MYSQL_API DSL_API_VIS
	#define DSL_MYSQL_API_CLASS DSL_API_VIS
#endif

/**
 * \defgroup mysql MySQL Database Wrapper
 */

/** \addtogroup mysql
 * @{
 */

class DSL_MYSQL_API_CLASS DB_MySQL: public SQLConx {
public:
	DB_MySQL();
	~DB_MySQL();

	bool Connect(const string& host, const string& user, const string& pass, const string& dbname, uint16_t port=0, const string& charset="");
	bool Connect();
	void Disconnect();
	bool Ping();

	string GetErrorString();
	unsigned int GetError();

	bool NoResultQuery(const string& query);
	MYSQL_RES *Query(const string& query);
	uint64_t NumRows(MYSQL_RES *result);
	bool FetchRow(MYSQL_RES *result, SC_Row& retRow);
	bool FreeResult(MYSQL_RES *result);

	uint32_t InsertID();
	uint64_t InsertID64();
	uint64_t AffectedRows();
	uint32_t GetQueryCount();

	string EscapeString(const string& str);
	MYSQL * GetHandle();

	SQLConxMulti * MultiStart();
	bool MultiSend(SQLConxMulti *);
	bool MultiEnd(SQLConxMulti *);

private:
	MYSQL * sql;
	string host, user, pass, dbname, charset;
	uint16_t port;
	uint32_t query_count = 0;
};

/**@}*/

#endif // _INCLUDE_SQLCONX_H_
