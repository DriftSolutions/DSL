//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DB_COMMON_H__
#define __DB_COMMON_H__

class DSL_API_CLASS SC_Row {
public:
	map<string, string> Values;
	uint32_t NumFields;

	SC_Row();
	~SC_Row();
	void Reset();

	string Get(string fieldname, string sDefault = "");
};

class DSL_API_CLASS SQLConxMulti {
public:
	~SQLConxMulti();

	void Query(std::string query);
	size_t Count();

	std::vector<std::string> queries;
};

class DSL_API_CLASS SQLConx {
public:
	typedef	int(*sql_printf_type)(const char * fmt, ...);
	void SetOutput(sql_printf_type pprintf = NULL);

	virtual SQLConxMulti * MultiStart() = 0;
	virtual bool MultiSend(SQLConxMulti *) = 0;
	virtual bool MultiEnd(SQLConxMulti *) = 0;
	/* used for multi interface, not for use with SELECT queries or others that return data */
	virtual void SCM_Query(std::string query, uint32_t len = 0) = 0;

protected:
	sql_printf_type sql_printf = NULL;
};

#endif // __DB_COMMON_H__
