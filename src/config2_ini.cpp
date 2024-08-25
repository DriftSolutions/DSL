#include <drift/dslcore.h>
#include <drift/config.h>
#include <drift/config2.h>
#include <drift/GenLib.h>
#include <math.h>

/**
 * Read in an INI file into a ConfigSection
 */
bool ConfigSection::loadFromStringINI(const char ** pconfig, size_t& line, const char * fn) {
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
		if (strlen(buf) < 3) {
			// the minimum meaningful line would be 3 chars long, specifically [x] or x=y
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

		if (strnicmp(buf, "#include ", 9) == 0) {
			char * p = &buf[9];
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
						if (!loadFromStringINI(&tmpc, tmpln, p)) {
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

		if (buf[0] == '[' && buf[strlen(buf)-1] == ']') { // open a new section
			buf[strlen(buf) - 1] = 0;
			const char * new_name = &buf[1];
			ConfigSection * sub;
			if (parent != NULL) {
				sub = parent->FindOrAddSection(new_name);
			} else {
				sub = FindOrAddSection(new_name);
			}
			if (sub == NULL) {
				printf("ERROR: Error finding or creating section '%s'\n", &buf[1]);
				break;
			}
			if (!sub->loadFromStringINI(&config, line, fn)) {
				break;
			}
			continue;
		}

		char * value = strchr(buf, '=');
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

void ConfigSection::writeSectionINI(stringstream& sstr, int level, bool single) const {
	sstr << "[" << name << "]\n";

	for (auto& x : values) {
		sstr << x.first << " = " << x.second->AsString() << "\n";
	};

	sstr << "\n";

	if (!single) {
		for (auto& x : sections) {
			x.second->writeSectionINI(sstr, level + 1);
		}
	}
}
