//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef ENABLE_MYSQL
#include <drift/dslcore.h>
#include <drift/DB_MySQL.h>
#if !defined(MARIADB_BASE_VERSION) && !defined(MARIADB_VERSION_ID) && MYSQL_VERSION_ID >= 80001 && MYSQL_VERSION_ID != 80002
typedef bool my_bool;
#endif
#include <sstream>
using namespace std;

bool dsl_mysql_init() {
	return (mysql_library_init(0, NULL, NULL) == 0);
}

void dsl_mysql_cleanup() {
	mysql_library_end();
}

DSL_LIBRARY_FUNCTIONS dsl_mysql_funcs = {
	false,
	DSL_OPTION_MYSQL,
	dsl_mysql_init,
	dsl_mysql_cleanup,
	NULL,
};
DSL_Library_Registerer dsl_mysql_autoreg(dsl_mysql_funcs);

#ifdef _WIN32
#define SC_STRCPY strncpy
#define SC_STRICMP _stricmp
#else
#define SC_STRCPY strlcpy
#define SC_STRICMP strcasecmp
#endif

DB_MySQL::DB_MySQL() {
	sql_printf = printf;
	query_count = 0;
	sql = NULL;
	port = 3306;
}


DB_MySQL::~DB_MySQL() {
	Disconnect();
}

void DB_MySQL::Disconnect() {
	if (sql) {
		mysql_close(sql);
		sql = NULL;
	}
}

bool DB_MySQL::Connect() {
	Disconnect();

	sql = mysql_init(NULL);
	if (sql == NULL) {
		sql_printf("Error initializing MySQL: %s\n", GetErrorString().c_str());
		return false;
	}

	my_bool do_reconnect = true;
	mysql_options(sql, MYSQL_OPT_RECONNECT, &do_reconnect);

	if (!mysql_real_connect(sql, host.c_str(), user.c_str(), pass.c_str(), NULL, port, NULL, 0)) {
		sql_printf("Error connecting to MySQL: %s\n", GetErrorString().c_str());
		mysql_close(sql);
		sql = NULL;
		return false;
	}

	if (dbname.length()) {
		if (mysql_select_db(sql, dbname.c_str()) != 0) {
			//mysql_real_query(sql, dbname);
			string query = "CREATE DATABASE ";
			query += dbname;
			mysql_real_query(sql, query.c_str(), query.length());
			if (mysql_select_db(sql, dbname.c_str()) == 0) {
				sql_printf("Created database '%s'\n", dbname);
			} else {
				sql_printf("Error connecting to MySQL: could not select or create and select DB!\n");
				mysql_close(sql);
				sql = NULL;
				return false;
			}
		}
	}

	if (charset.length()) {
		sql_printf("Set character set to: %s\n", charset.c_str());
		string query = "SET NAMES '";
		query += charset;
		query += "'";
		Query(query.c_str());
	}

	sql_printf("Connected!\n");
	sql_printf("MySQL Client Version: %s\n", mysql_get_client_info());
	sql_printf("MySQL Server Version: %s\n", mysql_get_server_info(sql));
	sql_printf("Connected to %s\n", mysql_get_host_info(sql));
	//sql_printf("%s\n", mysql_info(sql));
	return true;
}

bool DB_MySQL::Connect(std::string pHost, std::string pUser, std::string pPass, std::string pDBName, uint16_t lPort, std::string pCharset) {
	host = pHost;
	user = pUser;
	pass = pPass;
	dbname = pDBName;
	charset = pCharset;
	port = lPort;

	return Connect();
}

int DB_MySQL::Ping() {
	if (!sql) {
		sql_printf("sql error: you are not connected to a MySQL server!\n");
		sql_printf("Attempting auto-reconnect...\n");
		if (Connect()) {
			return mysql_ping(sql);
		}
		return -1;
	}
	int ret = mysql_ping(sql);
	if (ret != 0) {//auto-attempt reconnect
		sql_printf("Attempting auto-reconnect...\n");
		if (Connect()) {
			return mysql_ping(sql);
		}
		return ret;
	}
	return ret;
}

uint32_t DB_MySQL::InsertID() {
	return uint32_t(mysql_insert_id(sql));
}

uint64_t DB_MySQL::InsertID64() {
	return uint64_t(mysql_insert_id(sql));
}

uint64_t DB_MySQL::AffectedRows() {
	return uint64_t(mysql_affected_rows(sql));
}


uint32_t DB_MySQL::GetQueryCount() {
	return query_count;
}

std::string DB_MySQL::GetErrorString() {
	if (sql == NULL) { return "Unknown error"; }
	return mysql_error(sql);
}

unsigned int DB_MySQL::GetError() {
	if (sql == NULL) { return 0; }
	return mysql_errno(sql);
}

MYSQL_RES *DB_MySQL::Query(std::string query, uint32_t len) {
	if (len == 0) { len = query.length(); }
	MYSQL_RES * ret = NULL;

	if (!sql) {
		sql_printf("sql error: you are not connected to a MySQL server!\n");
		if (!Connect()) {
			return NULL;
		}
	}

	query_count++;
	if (!mysql_real_query(sql,query.c_str(),len)) {
		ret = mysql_store_result(sql);
	} else {
		sql_printf("sql error in query(%u): %s\n",GetError(),GetErrorString().c_str());
		if (len <= 4096) {
			sql_printf("Query: %s\n",query.c_str());
		} else {
			sql_printf("Query: <too large to print>\n");
		}
	}

	return ret;
}

bool DB_MySQL::FetchRow(MYSQL_RES *result, SC_Row& retRow) {
	MYSQL_ROW row;
	MYSQL_FIELD *fields;
	unsigned int num_fields;
	unsigned int i;

	retRow.Reset();

	num_fields = mysql_num_fields(result);
	if ((row = mysql_fetch_row(result))) {
		unsigned long * lengths = mysql_fetch_lengths(result);
		for(i = 0; i < num_fields; i++)	{
			fields = mysql_fetch_field_direct(result,i);

			if (lengths[i] > 0) {
				string str;
				str.assign(row[i], lengths[i]);
				retRow.Values[fields->name] = str;
			} else {
				retRow.Values[fields->name] = "";
			}
		}
	} else {
		return false;
	}

	retRow.NumFields = num_fields;
	return true;
}

uint64_t DB_MySQL::NumRows(MYSQL_RES *result) {
	if (!result) { return 0; }
	uint64_t ret = mysql_num_rows(result);
	return ret;
}

bool DB_MySQL::FreeResult(MYSQL_RES *result) {
	if (result == NULL) { return false; }
	mysql_free_result(result);
	return true;
}

std::string DB_MySQL::EscapeString(std::string str) {
	std::string ret = "";
	char * tmp = (char *)dsl_malloc((str.length() * 2) + 1);
	if (sql) {
		mysql_real_escape_string(sql, tmp, str.c_str(), str.length());
	} else {
		mysql_escape_string(tmp, str.c_str(), str.length());
	}
	ret = tmp;
	dsl_free(tmp);
	return ret;
}

MYSQL * DB_MySQL::GetHandle() { return sql; }

SQLConxMulti * DB_MySQL::MultiStart() {
	return new SQLConxMulti;
}


bool DB_MySQL::MultiEnd(SQLConxMulti * scm) {
	bool ret = MultiSend(scm);
	delete scm;
	return ret;
}

bool DB_MySQL::MultiSend(SQLConxMulti * scm) {
	if (!sql) {
		sql_printf("sql error: you are not connected to a MySQL server!\n");
		if (!Connect()) {
			return false;
		}
	}

	if (scm->Count() == 0) {
		return true;
	}

	if (mysql_set_server_option(sql, MYSQL_OPTION_MULTI_STATEMENTS_ON) != 0) {
		sql_printf("sql error: could not enable multi-statements!\n");
		return false;
	}

	bool ret = true;
	scm->queries.insert(scm->queries.begin(), "BEGIN;");
	scm->queries.push_back("COMMIT;");

	uint32_t tcount = 0;
	while (ret && scm->queries.size()) {
		stringstream q;
		int cnt=0;
		for (vector<string>::iterator x = scm->queries.begin(); x != scm->queries.end(); x = scm->queries.begin()) {
			if (q.str().length() == 0 || (q.str().length() + x->length()) < (1024*64)) {
				cnt++;
				q << *x;
				scm->queries.erase(x);
			} else {
				break;
			}
		}

		tcount += cnt;
		if (tcount >= 1000 && scm->queries.size()) {
#if defined(DEBUG)
			sql_printf("New transaction %d\n", tcount);
#endif
			q << "COMMIT;BEGIN;";
			tcount = 0;
		}

#if defined(DEBUG)
		sql_printf("Batch cnt %d\n", cnt);
#endif
		query_count += cnt;
		int status = mysql_real_query(sql, q.str().c_str(), q.str().length());
		/*
		FILE * fp2 = fopen("query.txt", "ab");
		if (fp2 != NULL) {
			fprintf(fp2, "%s\r\n", q.str().c_str());
			fclose(fp2);
		}
		*/
		if (status != 0) {
			sql_printf("sql error in query(%u): %s\n",GetError(),GetErrorString().c_str());
			ret = false;
			break;
		}

		do {
		  /* did current statement return data? */
			MYSQL_RES * result = mysql_store_result(sql);
			if (result) {
				mysql_free_result(result);
			} else {
				/* no result set or error */
				if (mysql_field_count(sql) != 0) {
				  sql_printf("sql error: could not retrieve result set\n");
				  ret = false;
				  break;
				}
			}
			/* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
			if ((status = mysql_next_result(sql)) > 0) {
				sql_printf("sql error: could not execute statement: %s\n",GetErrorString().c_str());
				ret = false;
			}
		} while (status == 0);
	}

	if (mysql_set_server_option(sql, MYSQL_OPTION_MULTI_STATEMENTS_OFF) != 0) {
		sql_printf("sql error: could not disable multi-statements!\n");
		return false;
	}

	return ret;
}

void DB_MySQL::SCM_Query(std::string query, uint32_t len) {
	MYSQL_RES * ret = Query(query, len);
	if (ret) {
		FreeResult(ret);
	}
}
#endif // ENABLE_MYSQL
