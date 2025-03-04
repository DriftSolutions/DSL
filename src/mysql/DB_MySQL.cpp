//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
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
#include <set>
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

DB_MySQL::DB_MySQL() {
	sql_printf = printf;
}


DB_MySQL::~DB_MySQL() {
	Disconnect();
}

void DB_MySQL::Disconnect() {
	if (sql != NULL) {
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
			mysql_real_query(sql, query.c_str(), (unsigned long)query.length());
			if (mysql_select_db(sql, dbname.c_str()) == 0) {
				sql_printf("Created database '%s'\n", dbname.c_str());
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
		NoResultQuery(query.c_str());
	}

	sql_printf("Connected!\n");
	sql_printf("MySQL Client Version: %s\n", mysql_get_client_info());
	sql_printf("MySQL Server Version: %s\n", mysql_get_server_info(sql));
	sql_printf("Connected to %s\n", mysql_get_host_info(sql));
	return true;
}

bool DB_MySQL::Connect(const string& pHost, const string& pUser, const string& pPass, const string& pDBName, uint16 lPort, const string& pCharset) {
	host = pHost;
	user = pUser;
	pass = pPass;
	dbname = pDBName;
	charset = pCharset;
	port = lPort;

	return Connect();
}

bool DB_MySQL::Ping() {
	if (sql == NULL) {
		sql_printf("sql error: you are not connected to a MySQL server!\n");
		sql_printf("Attempting auto-reconnect...\n");
		return Connect();
	}
	return (mysql_ping(sql) == 0);
}

uint32 DB_MySQL::InsertID() {
	uint64 ret = InsertID64();
	if (ret > UINT32_MAX) {
		// overflows a 32-bit unsigned int
		return 0;
	}
	return uint32(ret);
}

uint64 DB_MySQL::InsertID64() {
	return uint64(mysql_insert_id(sql));
}

uint64 DB_MySQL::AffectedRows() {
	my_ulonglong ret = mysql_affected_rows(sql);
	if (ret == (my_ulonglong)-1) {
		return 0;
	}
	return uint64(ret);
}

uint32 DB_MySQL::GetQueryCount() {
	return query_count;
}

string DB_MySQL::GetErrorString() {
	if (sql == NULL) {
		static const char errmsg[] = "Unknown error";
		return errmsg;
	}
	return mysql_error(sql);
}

unsigned int DB_MySQL::GetError() {
	if (sql == NULL) { return 0; }
	return mysql_errno(sql);
}

bool DB_MySQL::NoResultQuery(const string& query) {
	if (sql == NULL) {
		sql_printf("sql error: you are not connected to a MySQL server!\n");
		if (!Connect()) {
			return false;
		}
	}

	query_count++;
	bool ret = (mysql_real_query(sql, query.c_str(), (unsigned long)query.length()) == 0);
	if (!ret) {
		sql_printf("sql error in query(%u): %s\n", GetError(), GetErrorString().c_str());
		if (query.length() <= 4096) {
			sql_printf("Query: %s\n", query.c_str());
		} else {
			sql_printf("Query (truncated): %s ...\n", query.substr(0, 512).c_str());
		}
	}
	return ret;
}

MYSQL_RES *DB_MySQL::Query(const string& query) {
	if (!NoResultQuery(query)) {
		return NULL;
	}

	return mysql_store_result(sql);
}

static const set<enum_field_types> mysql_int_types = {
	MYSQL_TYPE_TINY, MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG, MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24
};
static const set<enum_field_types> mysql_float_types = {
	MYSQL_TYPE_DECIMAL, MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE, MYSQL_TYPE_NEWDECIMAL
};
static const set<enum_field_types> mysql_blob_types = {
	MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_BLOB
};

bool DB_MySQL::FetchRow(MYSQL_RES *result, SC_Row& retRow) {
	retRow.Reset();

	unsigned int num_fields = mysql_num_fields(result);
	MYSQL_ROW row;
	if ((row = mysql_fetch_row(result))) {
		unsigned long * lengths = mysql_fetch_lengths(result);
		for (unsigned int i = 0; i < num_fields; i++)	{
			MYSQL_FIELD * field = mysql_fetch_field_direct(result,i);
			if (lengths[i] > 0) {
				string str;
				str.assign(row[i], lengths[i]);

				DS_Value val;
				if (mysql_int_types.find(field->type) != mysql_int_types.end()) {
					val.SetValue((int64)atoi64(str.c_str()));
				} if (mysql_float_types.find(field->type) != mysql_float_types.end()) {
					val.SetValue(atof(str.c_str()));
				} if (mysql_blob_types.find(field->type) != mysql_blob_types.end()) {
					val.SetValue((const uint8 *)row[i], lengths[i]);
				} else {
					val.SetValue(str);
				}
				retRow.Values[field->name] = val;
			} else {
				retRow.Values[field->name] = "";
			}
		}
	} else {
		return false;
	}

	return true;
}

uint64 DB_MySQL::NumRows(MYSQL_RES *result) {
	if (result == NULL) { return 0; }
	return uint64(mysql_num_rows(result));
}

bool DB_MySQL::FreeResult(MYSQL_RES *result) {
	if (result == NULL) { return false; }
	mysql_free_result(result);
	return true;
}

string DB_MySQL::EscapeString(const string& str) {
	string ret;
	char * tmp = (char *)dsl_malloc((str.length() * 2) + 1);
	if (sql != NULL) {
		mysql_real_escape_string(sql, tmp, str.c_str(), (unsigned long)str.length());
	} else {
		mysql_escape_string(tmp, str.c_str(), (unsigned long)str.length());
	}
	ret = tmp;
	dsl_free(tmp);
	return ret;
}

MYSQL * DB_MySQL::GetHandle() { return sql; }

SQLConxMulti * DB_MySQL::MultiStart() {
	return new SQLConxMulti();
}


bool DB_MySQL::MultiEnd(SQLConxMulti * scm) {
	bool ret = MultiSend(scm);
	delete scm;
	return ret;
}

bool DB_MySQL::MultiSend(SQLConxMulti * scm) {
	if (sql == NULL) {
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

	uint32 tcount = 0;
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
		int status = mysql_real_query(sql, q.str().c_str(), (unsigned long)q.str().length());
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
			if (result != NULL) {
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

#endif // ENABLE_MYSQL
