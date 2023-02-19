//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
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

class DSL_API_CLASS ConfigValue {
	friend class Universal_Config2;
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

	int64 AsInt();
	double AsFloat();
	string AsString();
	bool AsBool(); /* true if it is an int > 0, double >= 1.00, or the strings "true" or "on" */

	void SetValue(const ConfigValue& val);
	void SetValue(int64 val);
	void SetValue(double val);
	void SetValue(const char * val);
	void SetValue(const uint8_t * val, size_t len);
	void SetValue(bool val);

	void Reset();
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
public:
	char name[64];
	typedef std::map<string, ConfigValue, uc_less> valueList;
	valueList values;
	typedef std::map<string, ConfigSection, uc_less> sectionList;
	sectionList sections; // sub-sections
};

typedef std::vector<ConfigSection *> scanStackType2;

class DSL_API_CLASS Universal_Config2 {	
	private:
		ConfigSection::sectionList sections;

		scanStackType2 scanStack;
		//ConfigSection *LastScan[20];

		void FreeSection(ConfigSection * Scan);
		void WriteSection(stringstream& sstr, ConfigSection * sec, int level);
		void WriteBinarySection(FILE * fp, ConfigSection * sec);
		void PrintSection(ConfigSection * Scan, int level);
		bool IsBool(const char * buf, bool * val = NULL);
		bool IsInt(const char * text);
		bool IsFloat(const char * text);
		ConfigSection * GetSectionFromString(const char * sec, bool create=false);

		ConfigSection * PopScan();
		void PushScan(ConfigSection * section);

	public:
		Universal_Config2();
		~Universal_Config2();

		bool LoadConfigFromString(const string config, const char * filename, ConfigSection * Scan = NULL);
		bool LoadConfigFromFile(const char * filename, ConfigSection * Scan=NULL);
		bool LoadConfigFromFile(FILE * fp, const char * filename, ConfigSection * Scan=NULL);
		bool WriteConfigToString(string& str, ConfigSection * Start = NULL, bool Single = false);
		bool WriteConfigToFile(const char * filename, ConfigSection * Start=NULL, bool Single=false);
		bool WriteConfigToFile(FILE * fp, ConfigSection * Start=NULL, bool Single=false);

		void FreeConfig();

		ConfigSection * GetSection(ConfigSection * parent, const char * name);
		ConfigValue GetSectionValue(ConfigSection * sec, const char * name);
		bool GetSectionValue(ConfigSection * sec, const char * name, ConfigValue& value);
		bool SectionHasValue(ConfigSection * sec, const char * name);
		void SetSectionValue(ConfigSection * sec, const char * name, const ConfigValue& val);

		// advanced commands
		ConfigSection * FindOrAddSection(ConfigSection * parent, const char * name);

		void ClearSection(ConfigSection * Scan);
		void PrintConfigTree();
};

#endif // __UNIVERSAL_CONFIG2_H__
