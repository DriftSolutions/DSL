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
#include <drift/config.h>
#include <drift/config2.h>
#include <drift/GenLib.h>
#include <math.h>

/*
ConfigSection::ConfigSection() {
}
*/

ConfigSection::~ConfigSection() {
	Clear();
}

/*
void ConfigSection::ClearSection(ConfigSection * Scan) {
	Scan->values.clear();
	Scan->sections.clear();
}

void ConfigSection::FreeSection(ConfigSection * Scan) {
	ClearSection(Scan);
};
*/

void ConfigSection::Clear() {
	for (auto& x : sections) {
		delete x.second;
	}
	sections.clear();
	for (auto& x : values) {
		delete x.second;
	}
	values.clear();
}

ConfigSection * ConfigSection::GetSection(const string& name) {
	auto x = sections.find(name);
	if (x != sections.end()) {
		return x->second;
	}
	return NULL;
}

bool ConfigSection::HasValue(const string& name) const {
	return (values.count(name) > 0);
	//return (values.find(name) != values.end());
}

/*
ConfigValue ConfigSection::GetValue(const string& name) {
	auto x = values.find(name);
	if (x != values.end()) {
		return *x->second;
	}
	return ConfigValue();
}
*/

const ConfigValue * ConfigSection::GetValue(const string& name) const {
	auto x = values.find(name);
	if (x != values.end()) {
		return x->second;
	}
	return NULL;
}

bool ConfigSection::GetValue(const string& name, ConfigValue& value) const {
	auto x = values.find(name);
	if (x != values.end()) {
		value = *x->second;
		return true;
	}
	return false;
}

void ConfigSection::SetValue(const string& name, const ConfigValue& val) {
	auto x = values.find(name);
	if (x != values.end()) {
		*x->second = val;
	} else {
		values[name] = new ConfigValue(val);
	}
}

bool ConfigValue::isBool(const char * buf, bool * val) {
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

bool ConfigValue::isInt(const char * buf) {
	if (buf == NULL || buf[0] == 0) { return false; } // null/empty string

	int64 l = atoi64(buf);
	if (l == 0 && strcmp(buf,"0")) {
		return false;
	}
	if (l == INT64_MAX && stricmp(buf, "9223372036854775807")) {
		return false;
	}
	if (l == INT64_MIN && stricmp(buf, "-9223372036854775807") && stricmp(buf, "-9223372036854775808")) {
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

bool ConfigValue::isFloat(const char * buf) {
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
			//Allow a negative sign in first position as long as more follows it
			continue;
		}
		if (buf[i] == '.') {
			// Allow one period
			periodCnt++;
			if (periodCnt == 1) { continue; }
			return false;
		}
		if (buf[i] < 48 || buf[i] > 57) {
			// Not a number
			return false;
		}
	}

	return (periodCnt == 1) ? true:false;
}

ConfigSection * ConfigSection::FindOrAddSection(const string& name) {
	ConfigSection * ret = GetSection(name);
	if (ret != NULL) {		
		return ret;
	}

	auto * s = new ConfigSection();
	s->name = name;
	sections[name] = s;
	return s;
}

bool ConfigSection::LoadFromString(const string& config, const string& fn) {
	const char * p = config.c_str();
	size_t line = 0;
	return loadFromString(&p, line, fn.c_str());
}

bool ConfigSection::loadFromString(const char ** pconfig, size_t& line, const char * fn) {
	bool long_comment = false;
	char buf[256] = { 0 };

	const char * config = *pconfig;
	while (*config != 0) {
		const char * eol = strchr(config, '\n');
		if (eol != NULL) {
			size_t len = eol - config + 1;
			strlcpy(buf, config, len);
			config += len;
		} else {
			sstrcpy(buf, config);
			config += strlen(config);
		}
		line++;
		strtrim(buf, " \t\r\n "); // first, trim the string of unwanted chars
		if (strlen(buf) < 2) {
			// the minimum meaningful line would be 2 chars long, specifically };
			continue;
		}

		str_replaceA(buf, sizeof(buf), "\t", " "); // turn tabs into spaces
		while (strstr(buf, "  ")) {
			str_replaceA(buf, sizeof(buf), "  ", " "); // turn double-spaces into spaces
		}

		//printf("line: %s\n",buf);

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
					string data;
					if (file_get_contents(p, data)) {
						const char * tmpc = data.c_str();
						size_t tmpln = 0;
						if (!loadFromString(&tmpc, tmpln, p)) {
							printf("ERROR: Error loading #included file '%s'\n", p);
							break;
						}
					} else {
						printf("ERROR: Error reading #included file '%s'\n", p);
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
			auto sub = FindOrAddSection(buf);
			if (!sub->loadFromString(&config, line, fn)) {
				break;
			}
			continue;
		}

		if (!strcmp(buf, "};")) { // close section
			break;
		}
		
		char * value = strchr(buf, ' ');
		if (value == NULL) {
			continue;
		}
		*value++ = 0;
		strtrim(buf);
		strtrim(value);
		if (buf[0] && value[0]) {
			auto x = values.find(buf);
			if (x != values.end()) {
				x->second->ParseString(value);
			} else {
				auto tmpv = new ConfigValue();
				tmpv->ParseString(value);
				values[buf] = tmpv;
			}
			continue;
		}

		printf("Unrecognized line at %s:%zu -> %s\n", fn, line, buf);
		// some unknown line here
	}

	*pconfig = config;
	return true;
}


bool ConfigSection::LoadFromFile(FILE * fp, const string& fn) {
	if (fp == NULL) { return false; }

	fseek64(fp, 0, SEEK_END);
	int64 len = ftell64(fp);
	if (len <= 0) { return false; }
	fseek64(fp, 0, SEEK_SET);

	bool ret = false;
	char * tmp = (char *)dsl_malloc(len + 1);
	if (fread(tmp, len, 1, fp) == 1) {
		tmp[len] = 0;
		ret = LoadFromString(tmp, fn);
	}
	dsl_free(tmp);
	return ret;
}

bool ConfigSection::LoadFromFile(const string& filename) {
	FILE * fp = fopen(filename.c_str(), "rb");
	if (fp == NULL) { return false; }
	bool ret = LoadFromFile(fp, filename);
	fclose(fp);
	return ret;
}

void ConfigSection::writeSection(stringstream& sstr, int level, bool single) const {
	char * pref = (char *)dsl_malloc(level+1);
	for(int i=0; i<level; i++) { pref[i] = '\t'; }
	pref[level]=0;

	sstr << pref << name << " {\n";

	if (!single) {
		for (auto& x : sections) {
			x.second->writeSection(sstr, level + 1);
		}
	}

	for (auto& x : values) {
		sstr << "\t" << pref << x.first << " " << x.second->AsString() << "\n";
	};

	sstr << pref << "};\n";
	dsl_free(pref);
}

bool ConfigSection::WriteToFile(const string& filename) const {
	FILE * fp = fopen(filename.c_str(), "wb");
	if (fp == NULL) { return false; }
	bool ret = WriteToFile(fp);
	fclose(fp);
	return ret;
}

bool ConfigSection::WriteToFile(FILE * fp) const {
	if (!fp) { return false; }
	string str = WriteToString();
	if (str.length() && fwrite(str.c_str(), str.length(), 1, fp) == 1) {
		return true;
	}
	return false;
}

string ConfigSection::WriteToString() const {
	stringstream sstr;
	for (auto& x : sections) {
		x.second->writeSection(sstr, 0);
	}
	//writeSection(sstr, 0, single);
	return sstr.str();
}

void ConfigSection::printSection(size_t level) const {
	char * pref = (char *)dsl_malloc(level + 1);
	for (int i = 0; i < level; i++) { pref[i] = '\t'; }
	pref[level] = 0;

	printf("%sSection: %s\n", pref, name.c_str());
	strcat(pref,"\t");
	for (auto& x : values) {
		switch(x.second->Type) {
			case DS_TYPE_INT:
				printf("%s[%s] = " I64FMT "\n", pref, x.first.c_str(), x.second->Int);
				break;
			case DS_TYPE_FLOAT:
				printf("%s[%s] = %f\n", pref, x.first.c_str(), x.second->Float);
				break;
			case DS_TYPE_STRING:
				printf("%s[%s] = '%s'\n", pref, x.first.c_str(), x.second->sString.c_str());
				break;
			case DS_TYPE_BINARY:
				printf("%s[%s] = Binary Data\n", pref, x.first.c_str());
				break;
			case DS_TYPE_BOOL:
				printf("%s[%s] = %s\n", pref, x.first.c_str(), (x.second->Int > 0) ? "true" : "false");
				break;
			default:
				printf("%s[%s] = Unknown Entry Type\n", pref, x.first.c_str());
				break;
		}
	}

	dsl_free(pref);

	for (auto& x : sections) {
		x.second->printSection(level+1);
	}
};

void ConfigSection::PrintConfigTree() const {
	printSection(0);
}

ConfigValue::ConfigValue() {
	Type = DS_TYPE_UNKNOWN;
	Int = 0;
}
ConfigValue::~ConfigValue() {
	Reset();
}

bool ConfigValue::AsBool() const {
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
int64 ConfigValue::AsInt() const {
	if (Type == DS_TYPE_STRING || Type == DS_TYPE_BINARY) {
		return atoi64(sString.c_str());
	} else if (Type == DS_TYPE_FLOAT) {
		return (int64)Float;
	} else if (Type == DS_TYPE_INT || Type == DS_TYPE_BOOL) {
		return Int;
	}
	return 0;
}
double ConfigValue::AsFloat() const {
	if (Type == DS_TYPE_STRING || Type == DS_TYPE_BINARY) {
		return atof(sString.c_str());
	} else if (Type == DS_TYPE_FLOAT) {
		return Float;
	} else if (Type == DS_TYPE_INT || Type == DS_TYPE_BOOL) {
		return (double)Int;
	}
	return 0;
}
string ConfigValue::AsString() const {
	if (Type == DS_TYPE_STRING || Type == DS_TYPE_BINARY) {
		return sString;
	} else if (Type == DS_TYPE_INT) {
		return mprintf("%lld", Int);
	} else if (Type == DS_TYPE_FLOAT) {
		return mprintf("%f", Float);
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
void ConfigValue::SetValue(const string& val) {
	Reset();
	Type = DS_TYPE_STRING;
	sString = val;
}
void ConfigValue::SetValue(const uint8_t * val, size_t len) {
	Reset();
	Type = DS_TYPE_BINARY;
	sString.assign((const char *)val, len);
}
void ConfigValue::SetValue(bool val) {
	Reset();
	Type = DS_TYPE_BOOL;
	Int = val;
}

void ConfigValue::ParseString(const char * value) {
	bool tmpbool = false;
	if (isFloat(value)) { // number
		SetValue(atof(value));
	} else if (isBool(value, &tmpbool)) { // bool
		SetValue(tmpbool);
	} else if (isInt(value)) { // number
		SetValue((int64)atoi64(value));
	} else { // string
		SetValue(value);
	}
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
