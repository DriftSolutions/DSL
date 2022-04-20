//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dslcore.h>
#include <drift/config.h>
#include <drift/config2.h>
#include <drift/GenLib.h>
#include <math.h>

Universal_Config2::Universal_Config2() {
}

Universal_Config2::~Universal_Config2() {
	FreeConfig();
}

void Universal_Config2::ClearSection(ConfigSection * Scan) {
	Scan->values.clear();
	Scan->sections.clear();
}

void Universal_Config2::FreeSection(ConfigSection * Scan) {
	ClearSection(Scan);
};

void Universal_Config2::FreeConfig() {
	sections.clear();
}

ConfigSection * Universal_Config2::GetSection(ConfigSection * parent, const char * name) {
	if (parent == NULL) {
		ConfigSection::sectionList::iterator x = sections.find(name);
		if (x != sections.end()) {
			return &x->second;
		}
	} else {
		ConfigSection::sectionList::iterator x = parent->sections.find(name);
		if (x != parent->sections.end()) {
			return &x->second;
		}
	}
	return NULL;
}

bool Universal_Config2::SectionHasValue(ConfigSection * sec, const char * name) {
	if (!sec) { return false; }
	return (sec->values.find(name) != sec->values.end());
}

ConfigValue Universal_Config2::GetSectionValue(ConfigSection * sec, const char * name) {
	ConfigValue rnull;
	if (!sec) { return rnull; }
	ConfigSection::valueList::iterator x = sec->values.find(name);
	if (x != sec->values.end()) {
		return x->second;
	}
	return rnull;
}

bool Universal_Config2::GetSectionValue(ConfigSection * sec, const char * name, ConfigValue& value) {
	if (!sec) { return false; }
	ConfigSection::valueList::iterator x = sec->values.find(name);
	if (x != sec->values.end()) {
		value = x->second;
		return true;
	}
	return false;
}

void Universal_Config2::SetSectionValue(ConfigSection * sec, const char * name, const ConfigValue& val) {
	if (!sec) { return; }
	sec->values[name] = val;
}

bool Universal_Config2::IsBool(const char * buf, bool * val) {
	if (buf == NULL || buf[0] == 0) { return false; } // null/empty string

	if (stricmp(buf, "true") == 0 || stricmp(buf, "on") == 0) {
		if (val != NULL) { *val = true; }
		return true;
	}
	if (stricmp(buf, "false") == 0 || stricmp(buf, "off") == 0) {
		if (val != NULL) { *val = false; }
		return true;
	}

	return false;
}

bool Universal_Config2::IsInt(const char * buf) {
	if (buf == NULL || buf[0] == 0) { return false; } // null/empty string

	int64 l = atoi64(buf);
	if (l == 0 && strcmp(buf,"0")) {
		return false;
	}
	if (l == LLONG_MAX && stricmp(buf, "9223372036854775807")) {
		return false;
	}
	if (l == LLONG_MIN && stricmp(buf, "-9223372036854775807") && stricmp(buf, "-9223372036854775808")) {
		return false;
	}

	//int periodCnt=0;
	for(int i=0; buf[i] != 0; i++) {
		if (buf[i] == '-' && i == 0 && buf[1] != 0) {
			continue;
		}
		if(buf[i] < 48 || buf[i] > 57) { // not a number
			return false;
		}
	}

	return true;
}

bool Universal_Config2::IsFloat(const char * buf) {
	if (buf == NULL || buf[0] == 0) { return false; } // null/empty string

	double l = atof(buf);
	if (l == 0.0 && strcmp(buf,"0.0") && strcmp(buf,"0.00")) {
		return false;
	}
	if (l == HUGE_VAL || l == -HUGE_VAL) {
		return false;
	}

	int periodCnt=0;
	for(int i=0; buf[i] != 0; i++) {
		if (buf[i] == '-' && i == 0 && buf[1] != 0) {
			continue;
		}
		if (buf[i] == '.') {
			periodCnt++;
			if (periodCnt == 1) { continue; }
			return false;
		}
		if(buf[i] < 48 || buf[i] > 57) { // not a number
			return false;
		}
	}

	return (periodCnt == 1) ? true:false;
}

ConfigSection * Universal_Config2::FindOrAddSection(ConfigSection * parent, const char * name) {
	ConfigSection * ret = GetSection(parent,name);
	if (!ret) {
		if (!parent) {
			strcpy(sections[name].name, name);
		} else {
			strcpy(parent->sections[name].name, name);
		}
		ret = GetSection(parent,name);
	}
	return ret;
}

ConfigSection * Universal_Config2::PopScan() {
	ConfigSection * ret = scanStack.back(); // LastScan[0];
	scanStack.pop_back();
	return ret;
}

void Universal_Config2::PushScan(ConfigSection * section) {
	scanStack.push_back(section);
}

/*
bool LoadConfigFromString(const string config, ConfigSection * Scan = NULL);
bool LoadConfigFromFile(const char * filename, ConfigSection * Scan = NULL);
bool WriteConfigToString(string& str, ConfigSection * Start = NULL, bool Single = false);
bool WriteConfigToFile(const char * filename, ConfigSection * Start = NULL, bool Single = false);
bool WriteConfigToFile(FILE * fp, ConfigSection * Start = NULL, bool Single = false);
*/

bool Universal_Config2::LoadConfigFromString(const string config, const char * fn, ConfigSection * Scan) {
	char buf[256];
	int line = 0;
	bool long_comment = false;
	memset(buf, 0, sizeof(buf));
	StrTokenizer st((char *)config.c_str(), '\n', true);

	for (int line=1; line <= st.NumTok(); line++) {
		sstrcpy(buf, st.stdGetSingleTok(line).c_str());
		strtrim(buf, " \t\r\n "); // first, trim the string of unwanted chars
		if (strlen(buf) < 2) {
			// the minimum meaningful line would be 2 chars long, specifically };
			continue;
		}

		str_replaceA(buf, sizeof(buf), "\t", " "); // turn tabs into spaces
		while (strstr(buf, "  ")) {
			str_replaceA(buf, sizeof(buf), "  ", " "); // turn double-spaces into spaces
		}

		//		printf("line: %s\n",buf);

		if (long_comment) {
			if (!strcmp(buf, "*/")) {
				long_comment = false;
			}
			continue;
		}

		if (!strncmp(buf, "/*", 2)) {
			long_comment = true;
			continue;
		}

		if (stristr(buf, "#include")) {
			char * p = stristr(buf, "#include");
			p = strchr(p, '\"');
			if (p) {
				p++;
				char *q = strchr(p, '\"');
				if (q) {
					q[0] = 0;
					if (LoadConfigFromFile(p, Scan)) {
						Scan = PopScan();
					} else {
						printf("ERROR: Error loading #included file '%s'\n", p);
						break;
					}
				} else {
					printf("ERROR: Syntax is #include \"filename\"\n");
					break;
				}
			} else {
				printf("ERROR: Syntax is #include \"filename\"\n");
				break;
			}
			continue;
		}

		if (buf[0] == '#' || !strncmp(buf, "//", 2)) {
			continue;
		}

		char *p = (char *)&buf + (strlen(buf) - 2); // will be " {" if beginning a section
		if (!strcmp(p, " {")) { // open a new section
			p[0] = 0;
			PushScan(Scan);
			Scan = FindOrAddSection(Scan, buf);
			continue;
		}

		if (!strcmp(buf, "};")) { // close section
			if (Scan != NULL) {
				Scan = PopScan();
			} else {
				printf("ERROR: You have one too many }; near line %d of %s\n", line, fn);
				break;
			}
			continue;
		}

		StrTokenizer tok(buf, ' ');
		unsigned long num = tok.NumTok();
		bool tmpbool = false;
		if (num > 1) {
			ConfigValue sVal;
			char * name = tok.GetSingleTok(1);
			char * value = tok.GetTok(2, num);
			if (IsFloat(value)) { // number
				sVal.SetValue(atof(value));
			} else if (IsBool(value, &tmpbool)) { // bool
				sVal.SetValue(tmpbool);
			} else if (IsInt(value)) { // number
				sVal.SetValue((int64)atoi64(value));
			} else { // string
				sVal.SetValue(value);
			}
			if (Scan) {
				SetSectionValue(Scan, name, sVal);
			}
			tok.FreeString(name);
			tok.FreeString(value);
			continue;
		}

		printf("Unrecognized line at %s:%d -> %s\n", fn, line, buf);
		// some unknown line here
	}

	PushScan(Scan);
	return true;
}


bool Universal_Config2::LoadConfigFromFile(FILE * fp, const char * fn, ConfigSection * Scan) {
	if (!fp) { return false; }

	fseek64(fp, 0, SEEK_END);
	int64 len = ftell64(fp);
	if (len <= 0) { return false; }
	fseek64(fp, 0, SEEK_SET);

	bool ret = false;
	char * tmp = (char *)dsl_malloc(len + 1);
	if (fread(tmp, len, 1, fp) == 1) {
		tmp[len] = 0;
		ret = LoadConfigFromString(tmp, fn, Scan);
	}
	dsl_free(tmp);
	return ret;
}

bool Universal_Config2::LoadConfigFromFile(const char * filename, ConfigSection * Scan) {
	FILE * fp = fopen(filename, "rb");
	if (!fp) { return false; }
	bool ret = LoadConfigFromFile(fp, filename, Scan);
	fclose(fp);
	return ret;
}

void Universal_Config2::WriteSection(stringstream& sstr, ConfigSection * sec, int level) {
	char * pref = (char *)dsl_malloc(level+1);
	for(int i=0; i<level; i++) { pref[i] = '\t'; }
	pref[level]=0;

	char buf[1024];
	sprintf(buf,"%s%s {\n", pref, sec->name);
	sstr << buf;

	for (ConfigSection::sectionList::iterator x = sec->sections.begin(); x != sec->sections.end(); x++) {
		WriteSection(sstr, &x->second, level+1);
	}

	for (ConfigSection::valueList::iterator x = sec->values.begin(); x != sec->values.end(); x++) {
		switch (x->second.Type) {
			case DS_TYPE_INT:
				sprintf(buf,"\t%s%s " I64FMT "\n",pref,x->first.c_str(),x->second.Int);
				sstr << buf;
				break;
			case DS_TYPE_STRING:
				sprintf(buf,"\t%s%s %s\n",pref,x->first.c_str(),x->second.sString.c_str());
				sstr << buf;
				break;
			case DS_TYPE_FLOAT:
				sprintf(buf,"\t%s%s %f\n",pref,x->first.c_str(),x->second.Float);
				sstr << buf;
				break;
			case DS_TYPE_BOOL:
				sprintf(buf,"\t%s%s %s\n",pref,x->first.c_str(),(x->second.Int > 0) ? "true" : "false");
				sstr << buf;
				break;
			default:
				break;
		}
	};

	sprintf(buf,"%s};\n",pref);
	sstr << buf;
	dsl_free(pref);
}

bool Universal_Config2::WriteConfigToFile(const char * filename, ConfigSection * Start, bool Single) {
	FILE * fp = fopen(filename,"wb");
	if (!fp) { return false; }
	bool ret = WriteConfigToFile(fp, Start, Single);
	fclose(fp);
	return ret;
}

bool Universal_Config2::WriteConfigToFile(FILE * fp, ConfigSection * Start, bool Single) {
	if (!fp) { return false; }
	string str;
	if (WriteConfigToString(str, Start, Single) && fwrite(str.c_str(), str.length(), 1, fp) == 1) {
		return true;
	}
	return false;
}

bool Universal_Config2::WriteConfigToString(string& str, ConfigSection * Start, bool Single) {
	stringstream sstr;
	if (Start == NULL) {
		for (ConfigSection::sectionList::iterator x = sections.begin(); x != sections.end(); x++) {
			WriteSection(sstr, &x->second, 0);
			sstr << "\n";
			if (Single) {
				break;
			}
		}
	} else {
		WriteSection(sstr, Start, 0);
	}
	str = sstr.str();
	return true;
}

void Universal_Config2::PrintSection(ConfigSection * Scan, int level) {
	char buf[32]={0};
	for (int i=0; i < level; i++) {
		strcat(buf,"\t");
	}

	printf("%sSection: %s\n",buf,Scan->name);
	strcat(buf,"\t");
	for (ConfigSection::valueList::iterator x = Scan->values.begin(); x != Scan->values.end(); x++) {
		switch(x->second.Type) {
			case DS_TYPE_INT:
				printf("%s[%s] = " I64FMT "\n",buf,x->first.c_str(),x->second.Int);
				break;
			case DS_TYPE_FLOAT:
				printf("%s[%s] = %f\n",buf,x->first.c_str(),x->second.Float);
				break;
			case DS_TYPE_STRING:
				printf("%s[%s] = '%s'\n",buf,x->first.c_str(),x->second.sString.c_str());
				break;
			case DS_TYPE_BINARY:
				printf("%s[%s] = Binary Data\n", buf, x->first.c_str());
				break;
			case DS_TYPE_BOOL:
				printf("%s[%s] = %s\n",buf,x->first.c_str(),(x->second.Int > 0) ? "true" : "false");
				break;
			default:
				printf("%s[%s] = Unknown Entry Type\n",buf,x->first.c_str());
				break;
		}
	}

	for (ConfigSection::sectionList::iterator x = Scan->sections.begin(); x != Scan->sections.end(); x++) {
		PrintSection(&x->second,level+1);
	}
};

void Universal_Config2::PrintConfigTree() {
	for (ConfigSection::sectionList::iterator x = sections.begin(); x != sections.end(); x++) {
		PrintSection(&x->second,0);
	}
}

ConfigSection * Universal_Config2::GetSectionFromString(const char * sec, bool create) {
	if (*sec == '/') { sec++; }
	StrTokenizer st((char *)sec, '/');
	ConfigSection * ret = NULL;

	int num = st.NumTok();
	for (int i=1; i <= num; i++) {
		char * str = st.GetSingleTok(i);
		if (strcmp(str, "")) {
			ret = create ? FindOrAddSection(ret, str) : GetSection(ret, str);
			st.FreeString(str);
			if (!ret) { return NULL; }
		}
	}
	return ret;
}

/*
ConfigValue Universal_Config2::GetValue(const char * ssec, const char * name) {
	ConfigSection * sec = GetSectionFromString(ssec);
	if (sec) {
		return GetSectionValue(sec, name);
	}
	ConfigValue rnull;
	return rnull;
}

string Universal_Config2::GetValueString(const char * sec, const char * name) {
	return GetValue(sec, name).AsString();
}

int64 Universal_Config2::GetValueInt(const char * sec, const char * name) {
	return GetValue(sec, name).AsInt();
}

bool Universal_Config2::GetValueBool(const char * sec, const char * name) {
	return GetValue(sec, name).AsBool();
}

double Universal_Config2::GetValueFloat(const char * sec, const char * name) {
	return GetValue(sec, name).AsFloat();
}

void Universal_Config2::SetValue(const char * ssec, const char * name, const ConfigValue& val) {
	ConfigSection * sec = GetSectionFromString(ssec, true);
	SetSectionValue(sec, name, val);
}
void Universal_Config2::SetValueString(const char * ssec, const char * name, const char * str) {
	ConfigSection * sec = GetSectionFromString(ssec, true);
	ConfigValue val;
	val.SetValue(str);
	SetSectionValue(sec, name, val);
}
void Universal_Config2::SetValueInt(const char * ssec, const char * name, int64 lval) {
	ConfigSection * sec = GetSectionFromString(ssec, true);
	ConfigValue val;
	val.SetValue(lval);
	SetSectionValue(sec, name, val);
}
void Universal_Config2::SetValueBool(const char * ssec, const char * name, bool lval) {
	ConfigSection * sec = GetSectionFromString(ssec, true);
	ConfigValue val;
	val.SetValue(lval);
	SetSectionValue(sec, name, val);
}
void Universal_Config2::SetValueFloat(const char * ssec, const char * name, double lval) {
	ConfigSection * sec = GetSectionFromString(ssec, true);
	ConfigValue val;
	val.SetValue(lval);
	SetSectionValue(sec, name, val);
}
void Universal_Config2::SetValueBinary(const char * ssec, const char * name, const uint8_t * data, size_t len) {
	ConfigSection * sec = GetSectionFromString(ssec, true);
	ConfigValue val;
	val.SetValue(data, len);
	SetSectionValue(sec, name, val);
}
*/

ConfigValue::ConfigValue() {
	Type = DS_TYPE_UNKNOWN;
	this->Int = 0;
}
ConfigValue::~ConfigValue() {
	Reset();
}

bool ConfigValue::AsBool() {
	//true if it is an int > 0, double >= 1.00, or the strings "true" or "on"
	if (Type == DS_TYPE_STRING) {
		if (stricmp(sString.c_str(), "true") == 0 || stricmp(sString.c_str(), "on") == 0) {
			return true;
		}
	} else if (Type == DS_TYPE_FLOAT) {
		return (Float >= 1.00);
	} else if (Type == DS_TYPE_INT || Type == DS_TYPE_BOOL) {
		return (Int > 0);
	}
	return false;
}
int64 ConfigValue::AsInt() {
	if (Type == DS_TYPE_STRING || Type == DS_TYPE_BINARY) {
		return atoi64(sString.c_str());
	} else if (Type == DS_TYPE_FLOAT) {
		return (int64)Float;
	} else if (Type == DS_TYPE_INT || Type == DS_TYPE_BOOL) {
		return Int;
	}
	return 0;
}
double ConfigValue::AsFloat() {
	if (Type == DS_TYPE_STRING || Type == DS_TYPE_BINARY) {
		return atof(sString.c_str());
	} else if (Type == DS_TYPE_FLOAT) {
		return Float;
	} else if (Type == DS_TYPE_INT || Type == DS_TYPE_BOOL) {
		return (double)Int;
	}
	return 0;
}
string ConfigValue::AsString() {
	if (Type == DS_TYPE_STRING || Type == DS_TYPE_BINARY) {
		return sString;
	} else if (Type == DS_TYPE_INT) {
		char buf[64];
		snprintf(buf, sizeof(buf), I64FMT, Int);
		return buf;
	} else if (Type == DS_TYPE_FLOAT) {
		char buf[64];
		snprintf(buf, sizeof(buf), "%f", Float);
		return buf;
	} else if (Type == DS_TYPE_BOOL) {
		return (Int > 0) ? "true" : "false";
	}
	return "";
}

void ConfigValue::SetValue(const ConfigValue& val) {
	Reset();
	Type = val.Type;
	if (val.Type == DS_TYPE_STRING || val.Type == DS_TYPE_BINARY) {
		sString = val.sString;
	} else {
		Int = val.Int;
	}
}
void ConfigValue::SetValue(int64 val) {
	Reset();
	Type = DS_TYPE_INT;
	Int = val;
}
void ConfigValue::SetValue(double val) {
	Reset();
	Type = DS_TYPE_FLOAT;
	Float = val;
}
void ConfigValue::SetValue(const char * val) {
	Reset();
	Type = DS_TYPE_STRING;
	sString = val;
}
void ConfigValue::SetValue(const uint8_t * val, size_t len) {
	Reset();
	Type = DS_TYPE_BINARY;
	string s(val, val + len);
	sString = s;
}
void ConfigValue::SetValue(bool val) {
	Reset();
	Type = DS_TYPE_BOOL;
	Int = val;
}

/*
void ConfigValue::Print(FILE * fp) {
}
*/

void ConfigValue::Reset() {
	Type = DS_TYPE_UNKNOWN;
	memset(&Int, 0, sizeof(Int));
	sString = "";
}
