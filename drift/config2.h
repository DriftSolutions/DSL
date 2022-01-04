//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
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
struct uc_less : binary_function<string, string, bool> {

  // case-independent (ci) compare_less binary function
  struct nocase_compare : public binary_function<unsigned char,unsigned char,bool>
    {
    bool operator() (const unsigned char& c1, const unsigned char& c2) const
      { return tolower (c1) < tolower (c2); }
    };

  bool operator() (const string & s1, const string & s2) const
    {

    return lexicographical_compare
          (s1.begin (), s1.end (),   // source range
           s2.begin (), s2.end (),   // dest range
                nocase_compare ());  // comparison
    }
}; // end of uc_less


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
		void WriteSection(FILE * fp, ConfigSection * sec, int level);
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

		bool LoadConfig(const char * filename, ConfigSection * Scan=NULL);
		bool LoadConfig(FILE * fp, const char * filename, ConfigSection * Scan=NULL);
		bool WriteConfig(const char * filename, ConfigSection * Start=NULL, bool Single=false);
		bool WriteConfig(FILE * fp, ConfigSection * Start=NULL, bool Single=false);

		void FreeConfig();

		ConfigSection * GetSection(ConfigSection * parent, const char * name);
		ConfigValue GetSectionValue(ConfigSection * sec, const char * name);
		bool GetSectionValue(ConfigSection * sec, const char * name, ConfigValue& value);
		bool SectionHasValue(ConfigSection * sec, const char * name);
		void SetSectionValue(ConfigSection * sec, const char * name, const ConfigValue& val);

		/*
		ConfigValue GetValue(const char * sec, const char * name);
		string GetValueString(const char * sec, const char * name);
		int64 GetValueInt(const char * sec, const char * name);
		bool GetValueBool(const char * sec, const char * name);
		double GetValueFloat(const char * sec, const char * name);

		void SetValue(const char * sec, const char * name, const ConfigValue& val);
		void SetValueString(const char * sec, const char * name, const char * val);
		void SetValueInt(const char * sec, const char * name, int64 val);
		void SetValueBool(const char * ssec, const char * name, bool lval);
		void SetValueFloat(const char * sec, const char * name, double val);
		void SetValueBinary(const char * sec, const char * name, const uint8_t * val, size_t len);
		*/

		// advanced commands
		ConfigSection * FindOrAddSection(ConfigSection * parent, const char * name);

		void ClearSection(ConfigSection * Scan);
		void PrintConfigTree();
};

#endif // __UNIVERSAL_CONFIG2_H__
