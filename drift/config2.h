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

class DSL_API_CLASS ConfigSection {
protected:
	typedef map<string, ConfigValue *, uc_less> valueList;
	typedef map<string, ConfigSection *, uc_less> sectionList;

	valueList _values;
	sectionList _sections; // sub-sections

	virtual bool loadFromString(const char ** config, size_t& line, const char * fn);

	virtual void writeSection(stringstream& sstr, int level, bool single = false) const;
	void printSection(size_t level) const;
	/*
			void FreeSection(ConfigSection * Scan);
		void WriteBinarySection(FILE * fp, ConfigSection * sec);
	*/
public:
	string name;
	const sectionList& sections = _sections;
	const valueList& values = _values;

	void Clear();
	void PrintConfigTree() const;

	bool LoadFromString(const string& config, const string& filename);
	bool LoadFromFile(const string& filename);
	bool LoadFromFile(FILE * fp, const string& filename);
	string WriteToString() const;
	bool WriteToFile(const string& filename) const;
	bool WriteToFile(FILE * fp) const;

	ConfigSection * GetSection(const string& name);
	const ConfigValue * GetValue(const string& name) const;
	bool GetValue(const string& name, ConfigValue& value) const;
	bool HasValue(const string& name) const;
	void SetValue(const string& name, const ConfigValue& val);

	// advanced commands
	virtual ConfigSection * FindOrAddSection(const string& name);

	~ConfigSection();
};

class DSL_API_CLASS ConfigINI: public ConfigSection {
protected:
	ConfigINI * parent = NULL;
	virtual bool loadFromString(const char ** config, size_t& line, const char * fn);
	virtual void writeSection(stringstream& sstr, int level, bool single = false) const;
public:
	virtual ConfigINI * FindOrAddSection(const string& name);
};

#endif // __UNIVERSAL_CONFIG2_H__
