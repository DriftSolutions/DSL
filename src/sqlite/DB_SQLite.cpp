//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifdef ENABLE_SQLITE
#include <drift/dslcore.h>
#include <drift/DB_SQLite.h>
#include <sstream>
using namespace std;

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(lib, "sqlite3_d.lib")
#else
#pragma comment(lib, "sqlite3.lib")
#endif
#endif

bool dsl_sqlite_init() {
	sqlite3_config(SQLITE_CONFIG_SERIALIZED);
	if (sqlite3_threadsafe() == 0) {
		printf("DSL: SQLite is compiled with threading disabled!\n");
		return false;
	}
	return (sqlite3_initialize() == SQLITE_OK);
}

void dsl_sqlite_cleanup() {
	sqlite3_shutdown();
}

DSL_LIBRARY_FUNCTIONS dsl_sqlite_funcs = {
	false,
	DSL_OPTION_SQLITE,
	dsl_sqlite_init,
	dsl_sqlite_cleanup,
	NULL,
};
DSL_Library_Registerer dsl_sqlite_autoreg(dsl_sqlite_funcs);

DB_SQLite::DB_SQLite() {
	sql_printf = printf;
}

DB_SQLite::~DB_SQLite() {
	Close();
}

void DB_SQLite::Close() {
	if (handle) {
		sqlite3_close(handle);
		handle = NULL;
	}
}

bool DB_SQLite::IsOpen() {
	return (handle != NULL);
}

bool DB_SQLite::Open(const string& fn) {
	return OpenV2(fn, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE  | SQLITE_OPEN_FULLMUTEX);
}

bool DB_SQLite::OpenV2(const string& fn, int flags, const string& vfs) {
	Close();

	if (sqlite3_threadsafe() == 0) {
		sql_printf("SQLite is compiled with threading disabled!\n");
		return false;
	}

	if (sqlite3_open_v2(fn.c_str(), &handle, flags, vfs.length() ? vfs.c_str() : NULL) != SQLITE_OK) {
		sql_printf("Error opening SQLite DB: %s\n", GetErrorString().c_str());
		if (handle) {
			sqlite3_close(handle);
			handle = NULL;
		}
		return false;
	}

	sqlite3_busy_timeout(handle, 10000);
	return true;
}

uint32 DB_SQLite::InsertID() {
	sqlite3_int64 ret = sqlite3_last_insert_rowid(handle);
	if (ret > UINT32_MAX) {
		// overflows a 32-bit unsigned int
		return 0;
	}
	return uint32_t(ret);
}

int64 DB_SQLite::InsertID64() {
	return sqlite3_last_insert_rowid(handle);
}

int DB_SQLite::AffectedRows() {
	return sqlite3_changes(handle);
}

#if SQLITE_VERSION_NUMBER >= 3037000
int64 DB_SQLite::AffectedRows64() {
	return sqlite3_changes64(handle);
}
#endif

uint32 DB_SQLite::GetQueryCount() {
	return query_count;
}

string DB_SQLite::GetErrorString() {
	if (handle == NULL) {
		static const char errmsg[] = "Database not opened";
		return errmsg;
	}
	return sqlite3_errmsg(handle);
}

int DB_SQLite::GetError() {
	if (handle == NULL) { return SQLITE_ERROR; }
	return sqlite3_errcode(handle);
}

struct SQLITE_ROW_CB {
	DB_SQLite_Query_Row_Callback cb;
	void * user_ptr;
};

int db_sqlite3_row_cb(void * ptr, int ncols, char ** values, char ** cols) {
	SQLITE_ROW_CB * cb = (SQLITE_ROW_CB *)ptr;
	SC_Row row;
	for (int i = 0; i < ncols; i++) {
		row.Values[cols[i]] = (values[i] != NULL) ? values[i] : "NULL";
	}
	return !cb->cb(row, cb->user_ptr);
}

bool DB_SQLite::Query(const string& query, DB_SQLite_Query_Row_Callback cb, void * uptr) {
	SQLITE_ROW_CB row_cb = { cb, uptr };
	return (Query(query, db_sqlite3_row_cb, &row_cb) == SQLITE_OK);
}

int db_sqlite3_result_cb(void * ptr, int ncols, char ** values, char ** cols) {
	SQLite_Result * ret = (SQLite_Result *)ptr;
	SC_Row row;
	for (int i = 0; i < ncols; i++) {
		row.Values[cols[i]] = (values[i] != NULL) ? values[i] : "NULL";
	}
	ret->rows.push_back(row);
	return 0;
}

SQLite_Result * DB_SQLite::Query(const string& query) {
	SQLite_Result * ret = new SQLite_Result();
	if (Query(query, db_sqlite3_result_cb, ret) == SQLITE_OK) {
		return ret;
	}
	delete ret;
	return NULL;
}

int DB_SQLite::Query(const string& query, DB_SQLite_Query_Callback cb, void * uptr) {
	if (!handle) {
		sql_printf("sql error: you don't have an SQLite DB open!\n");
		return SQLITE_ERROR;
	}

	query_count++;
	char * errmsg = NULL;
	int n = sqlite3_exec(handle, query.c_str(), cb, uptr, &errmsg);

	if (n != SQLITE_OK) {
		if (errmsg) {
			sql_printf("sql error in query: %s\n", errmsg);
		} else {
			sql_printf("sql error in query(%u): %s\n", GetError(), GetErrorString().c_str());
		}
		if (query.length() <= 4096) {
			sql_printf("Query: %s\n",query.c_str());
		} else {
			sql_printf("Query: <too large to print>\n");
		}
	}

	if (errmsg) {
		sqlite3_free(errmsg);
	}

	return n;
}

bool DB_SQLite::NoResultQuery(const string& query, bool silent_errors) {
	if (!handle) {
		sql_printf("sql error: you don't have an SQLite DB open!\n");
		return false;
	}

	query_count++;
	char * errmsg = NULL;
	int n = sqlite3_exec(handle, query.c_str(), NULL, NULL, &errmsg);

	if (n != SQLITE_OK && !silent_errors) {
		if (errmsg) {
			sql_printf("sql error in query: %s\n", errmsg);
		} else {
			sql_printf("sql error in query(%u): %s\n", GetError(), GetErrorString().c_str());
		}
		if (query.length() <= 4096) {
			sql_printf("Query: %s\n", query.c_str());
		} else {
			sql_printf("Query: <too large to print>\n");
		}
	}

	if (errmsg != NULL) {
		sqlite3_free(errmsg);
	}

	return (n == SQLITE_OK);
}


bool DB_SQLite::_insert(const string& table, const SC_Row& row, const string& action) {
	stringstream q;
	q << action << " INTO " << table << "(";
	bool first = true;
	for (auto& x : row.Values) {
		if (first) {
			first = false;
		} else {
			q << ", ";
		}
		q << "`" << x.first << "`";
	}
	q << ") VALUES (";
	first = true;
	for (auto& x : row.Values) {
		if (first) {
			first = false;
		} else {
			q << ", ";
		}
		q << "'" << EscapeString(x.second.AsString()) << "'";
	}
	q << ")";
	return NoResultQuery(q.str());
}

bool DB_SQLite::Insert(const string& table, const SC_Row& row) {
	return _insert(table, row, "INSERT");
}

bool DB_SQLite::InsertIgnore(const string& table, const SC_Row& row) {
	return _insert(table, row, "INSERT IGNORE");
}

bool DB_SQLite::InsertOrUpdate(const string& table, const SC_Row& row) {
	stringstream q;
	q << "INSERT INTO " << table << "(";
	bool first = true;
	for (auto& x : row.Values) {
		if (first) {
			first = false;
		} else {
			q << ", ";
		}
		q << "`" << x.first << "`";
	}
	q << ") VALUES (";
	first = true;
	for (auto& x : row.Values) {
		if (first) {
			first = false;
		} else {
			q << ", ";
		}
		q << "'" << EscapeString(x.second.AsString()) << "'";
	}
	q << ") ON CONFLICT DO UPDATE SET ";
	first = true;
	for (auto& x : row.Values) {
		if (first) {
			first = false;
		} else {
			q << ", ";
		}
		q << "`" << x.first << "`='" << EscapeString(x.second.AsString()) << "'";
	}
	return NoResultQuery(q.str());
}

bool DB_SQLite::Replace(const string& table, const SC_Row& row) {
	return _insert(table, row, "REPLACE");
}

bool DB_SQLite::FetchRow(SQLite_Result *result, SC_Row& retRow) {
	if (result && result->ind < result->rows.size()) {
		retRow = result->rows[result->ind++];
		return true;
	} else {
		retRow.Reset();
	}
	return false;
}

bool DB_SQLite::FetchRow(SQLite_Result *result, SC_Row ** retRow) {
	if (result && result->ind < result->rows.size()) {
		*retRow = &result->rows[result->ind++];
		return true;
	}
	return false;
}

size_t DB_SQLite::NumRows(SQLite_Result *result) {
	if (result) {
		return result->rows.size();
	}
	return 0;
}

bool DB_SQLite::FreeResult(SQLite_Result *result) {
	if (result == NULL) { return false; }
	delete result;
	return true;
}

string DB_SQLite::EscapeString(const string& str) {
	char * tmp = sqlite3_mprintf("%q", str.c_str());
	string ret = tmp;
	sqlite3_free(tmp);
	return ret;
}

std::string DB_SQLite::MPrintf(const char * str, ...) {
	va_list ap;
	va_start(ap, str);
	char * tmp = sqlite3_vmprintf(str, ap);
	va_end(ap);
	string ret = tmp;
	sqlite3_free(tmp);
	return ret;
}

sqlite3 * DB_SQLite::GetHandle() { return handle; }

SQLConxMulti * DB_SQLite::MultiStart() {
	return new SQLConxMulti;
}


bool DB_SQLite::MultiEnd(SQLConxMulti * scm) {
	bool ret = MultiSend(scm);
	delete scm;
	return ret;
}

bool DB_SQLite::MultiSend(SQLConxMulti * scm) {
	if (!handle) {
		sql_printf("sql error: you don't have an SQLite DB open!\n");
		return false;
	}

	if (scm->Count() == 0) {
		return true;
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
		char * errmsg = NULL;
		int n = sqlite3_exec(handle, q.str().c_str(), NULL, NULL, &errmsg);
		if (n == SQLITE_OK) {
			if (errmsg) {
				sqlite3_free(errmsg);
			}
		} else {
			ret = false;
			if (errmsg) {
				sql_printf("sql error in query: %s\n", errmsg);
				sqlite3_free(errmsg);
			} else {
				sql_printf("sql error in query(%u): %s\n", GetError(), GetErrorString().c_str());
			}
		}
	}

	return ret;
}
#endif // ENABLE_SQLITE
