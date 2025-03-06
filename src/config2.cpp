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
	_sections.clear();
	for (auto& x : values) {
		delete x.second;
	}
	_values.clear();
}

ConfigSection * ConfigSection::GetSection(const string& name) {
	auto x = sections.lower_bound(name);
	if (x != sections.end()) {
		return x->second;
	}
/*		
	auto x = sections.find(name);
	if (x != sections.end()) {
		return x->second;
	}
	*/
	return NULL;
}

bool ConfigSection::GetSections(const string& name, vector<ConfigSection *>& psections) {
	psections.clear();
	auto matches = sections.equal_range(name);
	for (auto i = matches.first; i != matches.second; ++i) {
		psections.push_back(i->second);
	}
	return (psections.size() > 0);
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

const ConfigValue * const ConfigSection::GetValue(const string& name) const {
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
		_values[name] = new ConfigValue(val);
	}
}

ConfigSection * ConfigSection::FindOrAddSection(const string& name, bool force_new) {
	if (!force_new) {
		ConfigSection * ret = GetSection(name);
		if (ret != NULL) {
			return ret;
		}
	}

	auto * s = new ConfigSection();
	s->_name = name;
	s->parent = this;
	_sections.insert({ name,s });
	return s;
}

bool ConfigSection::LoadFromString(const string& config, const string& fn, DSL_CONFIG_FORMAT f, bool allow_duplicate_section_names) {
	const char * p = config.c_str();
	size_t line = 0;
	if (f == DCF_AUTO) {
		f = getSerializerModeFromFN(fn);
	}
	if (f == DCF_INI) {
		return loadFromStringINI(&p, line, fn.c_str(), allow_duplicate_section_names);
	} else {
		return loadFromStringConf(&p, line, fn.c_str(), allow_duplicate_section_names);
	}
}

DSL_CONFIG_FORMAT ConfigSection::getSerializerModeFromFN(const string& fn) const {
	auto ext = strrchr(fn.c_str(), '.');
	if (ext != NULL && !stricmp(ext, ".ini")) {
		return DCF_INI;
	}
	return DCF_CONF;
}

bool ConfigSection::loadFromStringConf(const char ** pconfig, size_t& line, const char * fn, bool allow_duplicate_section_names) {
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
						if (!loadFromStringConf(&tmpc, tmpln, p, allow_duplicate_section_names)) {
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
			auto sub = FindOrAddSection(buf, allow_duplicate_section_names);
			if (!sub->loadFromStringConf(&config, line, fn, allow_duplicate_section_names)) {
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
				_values[buf] = tmpv;
			}
			continue;
		}

		printf("Unrecognized line at %s:%zu -> %s\n", fn, line, buf);
		// some unknown line here
	}

	*pconfig = config;
	return true;
}


bool ConfigSection::LoadFromFile(FILE * fp, const string& fn, DSL_CONFIG_FORMAT f, bool allow_duplicate_section_names) {
	if (fp == NULL) { return false; }

	fseek64(fp, 0, SEEK_END);
	int64 len = ftell64(fp);
	if (len <= 0) { return false; }
	fseek64(fp, 0, SEEK_SET);

	bool ret = false;
	char * tmp = (char *)dsl_malloc(len + 1);
	if (fread(tmp, len, 1, fp) == 1) {
		tmp[len] = 0;
		ret = LoadFromString(tmp, fn, f, allow_duplicate_section_names);
	}
	dsl_free(tmp);
	return ret;
}

bool ConfigSection::LoadFromFile(const string& filename, DSL_CONFIG_FORMAT f, bool allow_duplicate_section_names) {
	FILE * fp = fopen(filename.c_str(), "rb");
	if (fp == NULL) { return false; }
	bool ret = LoadFromFile(fp, filename, f, allow_duplicate_section_names);
	fclose(fp);
	return ret;
}

void ConfigSection::writeSectionConf(stringstream& sstr, int level, bool single) const {
	char * pref = (char *)dsl_malloc(level+1);
	for(int i=0; i<level; i++) { pref[i] = '\t'; }
	pref[level]=0;

	sstr << pref << name << " {\n";

	if (!single) {
		for (auto& x : sections) {
			x.second->writeSectionConf(sstr, level + 1);
		}
	}

	for (auto& x : values) {
		sstr << "\t" << pref << x.first << " " << x.second->AsString() << "\n";
	};

	sstr << pref << "};\n";
	dsl_free(pref);
}

bool ConfigSection::WriteToFile(const string& filename, DSL_CONFIG_FORMAT f) const {
	FILE * fp = fopen(filename.c_str(), "wb");
	if (fp == NULL) { return false; }
	bool ret = WriteToFile(fp, filename, f);
	fclose(fp);
	return ret;
}

bool ConfigSection::WriteToFile(FILE * fp, const string& filename, DSL_CONFIG_FORMAT f) const {
	if (fp == NULL) { return false; }
	if (f == DCF_AUTO) {
		f = getSerializerModeFromFN(filename);
	}
	string str = WriteToString(f);
	if (str.length() && fwrite(str.c_str(), str.length(), 1, fp) == 1) {
		return true;
	}
	return false;
}

string ConfigSection::WriteToString(DSL_CONFIG_FORMAT f) const {
	stringstream sstr;
	for (auto& x : sections) {
		if (f == DCF_INI) {
			x.second->writeSectionINI(sstr, 0);
		} else {
			x.second->writeSectionConf(sstr, 0);
		}
	}
	//writeSection(sstr, 0, single);
	return sstr.str();
}

void ConfigSection::printSection(size_t level) const {
	char * pref = (char *)dsl_malloc(level + 2);
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

