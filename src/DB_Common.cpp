//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dslcore.h>
#include <drift/DB_Common.h>

SC_Row::SC_Row() {
}

SC_Row::~SC_Row() {
	Reset();
}

void SC_Row::Reset() {
	Values.clear();
}

string SC_Row::Get(const string& fieldname, const string& sDefault) const {
	auto x = Values.find(fieldname);
	if (x != Values.end()) {
		return x->second.AsString();
	}
	return sDefault;
}

SQLConxMulti::~SQLConxMulti() {
	queries.clear();
}

void SQLConxMulti::Query(const string& query) {
	queries.push_back(query + ";");
}

size_t SQLConxMulti::Count() {
	return queries.size();
}

void SQLConx::SetOutput(sql_printf_type pprintf) {
	sql_printf = (pprintf != NULL) ? pprintf : printf;
}
