//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2024 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __UNIVERSAL_CONFIG2_H__
#define __UNIVERSAL_CONFIG2_H__

/*
enum DS_VALUE_TYPE {
	DS_TYPE_LONG,
	DS_TYPE_FLOAT,
	DS_TYPE_STRING,
	DS_TYPE_BINARY
};
*/

class ConfigSection;

class DSL_API_CLASS ConfigValue {
	friend class ConfigSection;
private:
	bool isBool(const char * buf, bool * val = NULL);
	bool isInt(const char * text);
	bool isFloat(const char * text);

protected:
	string sString;
	union {
		int64 Int;
		double Float;
	};

public:
	ConfigValue();
	~ConfigValue();

	DS_VALUE_TYPE Type;

	int64 AsInt() const;
	double AsFloat() const;
	string AsString() const;
	bool AsBool() const; /* true if it is an int > 0, double >= 1.00, or the strings "true" or "on" */

	void SetValue(const ConfigValue& val);
	void SetValue(int64 val);
	void SetValue(double val);
	void SetValue(const char * val);
	void SetValue(const string& val);
	void SetValue(const uint8_t * val, size_t len);
	void SetValue(bool val);

	void Reset();
	void ParseString(const char * value);
};

// case-independent (ci) string less_than
// returns true if s1 < s2
#if __cplusplus >= 201103L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L)
struct uc_less {
	// case-independent (ci) compare_less binary function
	struct nocase_compare {
		bool operator() (const unsigned char& c1, const unsigned char& c2) const {
			return tolower(c1) < tolower(c2);
		}
	};
	bool operator() (const std::string & s1, const std::string & s2) const {
		return std::lexicographical_compare
		(s1.begin(), s1.end(),   // source range
			s2.begin(), s2.end(),   // dest range
			nocase_compare());  // comparison
	}
 };
#else
struct uc_less {

	// case-independent (ci) compare_less binary function
	struct nocase_compare : public binary_function<unsigned char, unsigned char, bool> {
		bool operator() (const unsigned char& c1, const unsigned char& c2) const {
			return tolower(c1) < tolower(c2);
		}
	};

	bool operator() (const string & s1, const string & s2) const {

		return lexicographical_compare
		(s1.begin(), s1.end(),   // source range
			s2.begin(), s2.end(),   // dest range
			nocase_compare());  // comparison
	}
}; // end of uc_less
#endif

enum DSL_CONFIG_FORMAT {
	DCF_AUTO, // use INI format if filename ends in .ini, CONF format otherwise
	DCF_CONF,
	DCF_INI
};

class DSL_API_CLASS ConfigSection {
protected:
	ConfigSection * parent = NULL;
	typedef map<string, ConfigValue *, uc_less> valueList;
	typedef map<string, ConfigSection *, uc_less> sectionList;

	string _name;
	valueList _values;
	sectionList _sections; // sub-sections

	DSL_CONFIG_FORMAT getSerializerModeFromFN(const string& filename) const;

	virtual bool loadFromStringConf(const char ** config, size_t& line, const char * fn);
	virtual void writeSectionConf(stringstream& sstr, int level, bool single = false) const;
	virtual bool loadFromStringINI(const char ** config, size_t& line, const char * fn);
	virtual void writeSectionINI(stringstream& sstr, int level, bool single = false) const;
	void printSection(size_t level) const;
public:
	const string& name = _name;
	const sectionList& sections = _sections;
	const valueList& values = _values;

	void Clear();
	void PrintConfigTree() const;

	bool LoadFromString(const string& config, const string& filename, DSL_CONFIG_FORMAT f = DCF_AUTO);
	bool LoadFromFile(const string& filename, DSL_CONFIG_FORMAT f = DCF_AUTO);
	bool LoadFromFile(FILE * fp, const string& filename, DSL_CONFIG_FORMAT f = DCF_AUTO);
	string WriteToString(DSL_CONFIG_FORMAT f) const;
	bool WriteToFile(const string& filename, DSL_CONFIG_FORMAT f = DCF_AUTO) const;
	bool WriteToFile(FILE * fp, const string& filename, DSL_CONFIG_FORMAT f = DCF_AUTO) const;

	ConfigSection * GetSection(const string& name);
	const ConfigValue * GetValue(const string& name) const;
	bool GetValue(const string& name, ConfigValue& value) const;
	bool HasValue(const string& name) const;
	void SetValue(const string& name, const ConfigValue& val);

	// advanced commands
	virtual ConfigSection * FindOrAddSection(const string& name);

	~ConfigSection();
};

#endif // __UNIVERSAL_CONFIG2_H__
