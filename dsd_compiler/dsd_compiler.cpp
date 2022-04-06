#include <drift/dsl.h>
#include "tokens.h"

map<string, DSD_TOKEN_ID> token_map;

#define ALLOWED_NAME_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789"
inline bool IsValidName(const string& name) {
	return (name.length() > 0 && strspn(name.c_str(), ALLOWED_NAME_CHARS) == name.length());
}

class DSD_Token {
public:
	DSD_Token(string pfn, int pline, DSD_TOKEN_ID pid, string tok, string val) {
		fn = pfn;
		line = pline;
		id = pid;
		token = tok;
		value_str = val;
	}
	DSD_Token(string pfn, int pline, DSD_TOKEN_ID pid, string tok, uint64 val) {
		fn = pfn;
		line = pline;
		id = pid;
		token = tok;
		value_int = val;
	}
	DSD_Token(string pfn, int pline, DSD_TOKEN_ID pid, string tok) {
		fn = pfn;
		line = pline;
		id = pid;
		token = tok;
	}
	DSD_Token() {
	}

	DSD_TOKEN_ID id = DTI_NONE;
	string fn;
	int line = 0;
	string token;
	string value_str;
	uint64 value_int = 0;
};

bool parse_file(string fn, vector<DSD_Token>& tokens) {
	FILE * fp = fopen(fn.c_str(), "rb");
	if (fp == NULL) {
		printf("Error opening %s for read!\n", fn.c_str());
		return false;
	}
	char buf[1024];
	string tok;
	int line = 0;
	while (!feof(fp) && (fgets(buf, sizeof(buf), fp) != NULL)) {
		line++;
		strtrim(buf);
		tok = "";
		char * p = buf;
		while (1) {
			if (*p == ' ' || *p == '\t' || (*p == ']' && tok.length()) || (*p == ';' && *(p+1) == 0) || *p == 0) {
				if (tok.length() > 0) {
					auto x = token_map.find(tok);
					if (x != token_map.end()) {
						DSD_Token tmp(fn, line, x->second, x->first);
						tokens.push_back(tmp);
					} else if (strspn(tok.c_str(), "0123456789") == tok.length()) {
						DSD_Token tmp(fn, line, DTI_NUMBER, "intval", atou64(tok.c_str()));
						tokens.push_back(tmp);
					} else {
						DSD_Token tmp(fn, line, DTI_STRING, "strval", tok);
						tokens.push_back(tmp);
					}
				}
				if (*p == ';' && *(p + 1) == 0) {
					DSD_Token tmp(fn, line, DTI_EOS, ";");
					tokens.push_back(tmp);
					break;
				} else if (*p == ']' && tok.length()) {
					DSD_Token tmp(fn, line, DTI_CLOSE_BRACKET, "]");
					tokens.push_back(tmp);
				} else if (*p == 0) {
					break;
				}
				tok = "";
				p++;
				continue;
			}
			tok += *p;
			p++;

			auto x = token_map.find(tok);
			if (x != token_map.end()) {
				DSD_Token tmp(fn, line, x->second, x->first);
				tokens.push_back(tmp);
				tok = "";
			}
		}
	}
	fclose(fp);
	return true;
};

inline bool IsDataType(DSD_TOKEN_ID type) {
	return (type >= DTI_TYPE_STRING && type < DTI_NUM_TOKENS);
}
inline bool IsNumericType(DSD_TOKEN_ID type) {
	return (type >= DTI_TYPE_UINT8 && type < DTI_NUM_TOKENS);
}

class DSD_Variable {
public:
	uint8 id = 0;
	string name;
	DSD_TOKEN_ID type = DTI_NONE;
	bool optional = false;
	bool vector = false;
	uint64 size = 0; // for varchar
	string custom_type;

	void Reset() {
		id = 0;
		name = "";
		type = DTI_NONE;
		optional = false;
		vector = false;
		size = 0;
		custom_type = "";
	}

	bool IsValid() {
		return (id > 0 && IsDataType(type));
	}
};

class DSD_Definition {
public:
	string name;
	vector<DSD_Variable> variables;
	set<uint8> used_ids;
	set<string> used_names;

	void Reset() {
		name = "";
		variables.clear();
		used_ids.clear();
		used_names.clear();
	}
};

inline bool get_token(vector<DSD_Token>& tokens, size_t i, DSD_Token& cur) {
	if (i >= tokens.size()) {
		return false;
	}
	cur = tokens[i];
	return true;
}

void output_header(FILE  * fp) {
	fprintf(fp, "/***********************************************************************\\\n");
	fprintf(fp, "|        Generated by DSL Serializer Definition (DSD) Compiler          |\n");
	fprintf(fp, "\\***********************************************************************/\n\n");
}

int main(int argc, const char * argv[]) {
	printf("DSL Serializer Definition (DSD) Compiler\n");
	printf("Copyright 2022 Drift Solutions. All rights reserved.\n\n");
#if defined(_WIN32)
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
	SetProcessDEPPolicy(PROCESS_DEP_ENABLE);
#endif
	SetConsoleTitle("DSD Compiler");
#endif

	dsl_init();
	atexit(dsl_cleanup);

	if (argc < 2) {
		printf("Usage: %s filename.dsd\n", nopath(argv[0]));
		exit(1);
	}

	string fn = argv[1];
	char * fntmp = strdup(argv[1]);
	char * ext = strrchr(fntmp, '.');
	if (ext == NULL) {
		printf("Could not get file extension!\n");
		free(fntmp);
		exit(1);
	}
	if (!stricmp(ext, ".cpp") || !stricmp(ext, ".h")) {
		printf("DSD file extension cannot be .cpp or .h!\n");
		free(fntmp);
		exit(1);
	}
	*ext = 0;
	string fn_cpp = fntmp;
	fn_cpp += ".cpp";
	string fn_h = fntmp;
	fn_h += ".h";
	free(fntmp);
	printf("DSD File: %s\n", fn.c_str());
	printf("CPP Ouput: %s\n", fn_cpp.c_str());
	printf("Header Ouput: %s\n", fn_h.c_str());

	for (int i = 0; dsd_tokens[i].id != DTI_NONE; i++) {
		token_map[dsd_tokens[i].token] = dsd_tokens[i].id;
	}
	vector<DSD_Token> tokens;
	if (!parse_file(fn, tokens)) {
		exit(1);
	}
	if (tokens.size() == 0) {
		printf("That does not appear to be a valid DSD file!\n");
		exit(1);
	}
#ifdef DEBUG
	printf("Parsed into %zu tokens...\n", tokens.size());
	/*
	for (auto x : tokens) {
		printf("%d[%s] => %s / " U64FMT "\n", x.id, x.token.c_str(), x.value_str.c_str(), x.value_int);
	}
	*/
#endif

	set<string> custom_types;
	vector<DSD_Definition> definitions;
	DSD_Definition def;
	DSD_Variable var;
	DSD_Token cur;
	size_t i = 0;
	while (get_token(tokens, i++, cur)) {
		if (def.name.length() == 0) {
			if (cur.id == DTI_DEF) {
				if (get_token(tokens, i++, cur) && cur.id == DTI_STRING) {
					string name = cur.value_str;
					if (IsValidName(name)) {
						if (get_token(tokens, i++, cur) && cur.id == DTI_OPEN_PAREN) {
							def.Reset();
							var.Reset();
							def.name = name;
						} else {
							printf("Syntax error at %s:%d, expected: def name {\n", cur.fn.c_str(), cur.line);
							exit(1);
						}
					} else {
						printf("Syntax error at %s:%d, invalid name '%s'. Allowed characters are: %s\n", cur.fn.c_str(), cur.line, name.c_str(), ALLOWED_NAME_CHARS);
						exit(1);
					}
				} else {
					printf("Syntax error at %s:%d, expected: def name {\n", cur.fn.c_str(), cur.line);
					exit(1);
				}
			} else {
				printf("Syntax error at %s:%d, expected: def name {\n", cur.fn.c_str(), cur.line);
				exit(1);
			}
		} else if (cur.id == DTI_CLOSE_PAREN) {
			if (def.variables.size() == 0) {
				printf("Syntax error at %s:%d, definition has no variables defined\n", cur.fn.c_str(), cur.line);
				exit(1);
			}
			definitions.push_back(def);
			custom_types.insert(def.name);
			def.Reset();
		} else if (var.type == DTI_NONE && cur.id == DTI_OPTIONAL) {
			var.optional = true;
		} else if (var.type == DTI_NONE && cur.id == DTI_VECTOR) {
			var.vector = true;
		} else if (var.type == DTI_NONE && (IsDataType(cur.id) || (cur.id == DTI_STRING && custom_types.find(cur.value_str) != custom_types.end()))) {
			if (cur.id == DTI_STRING) {
				var.type = DTI_TYPE_CUSTOM;
				var.custom_type = cur.value_str;
			} else {
				var.type = cur.id;
			}
			if (var.vector && (var.type != DTI_TYPE_STRING && var.type != DTI_TYPE_CUSTOM && !IsNumericType(var.type))) {
				printf("Syntax error at %s:%d, we can only vector integers, strings, and custom types atm.\n", cur.fn.c_str(), cur.line);
				exit(1);
			}
			if (cur.id == DTI_TYPE_CHAR || cur.id == DTI_TYPE_BYTES) {
				if (get_token(tokens, i++, cur) && cur.id == DTI_OPEN_BRACKET) {
					if (get_token(tokens, i++, cur) && cur.id == DTI_NUMBER && cur.value_int > 0) {
						var.size = cur.value_int;
						if (get_token(tokens, i++, cur) && cur.id == DTI_CLOSE_BRACKET) {
						} else {
							printf("Syntax error at %s:%d, expected ] after size of type char or bytes.\n", cur.fn.c_str(), cur.line);
							exit(1);
						}
					} else {
						printf("Syntax error at %s:%d, expected size > 0 after [.\n", cur.fn.c_str(), cur.line);
						exit(1);
					}
				} else {
					printf("Syntax error at %s:%d, expected [ after entry type char or bytes.\n", cur.fn.c_str(), cur.line);
					exit(1);
				}
			}
			if (get_token(tokens, i++, cur) && cur.id == DTI_STRING) {
				var.name = cur.value_str;
				if (IsValidName(var.name)) {
					if (def.used_names.find(var.name) != def.used_names.end()) {
						printf("Syntax error at %s:%d, variable name '%s' has already been used in this definition.\n", cur.fn.c_str(), cur.line, var.name.c_str());
						exit(1);
					}
					def.used_names.insert(var.name);
					if (get_token(tokens, i++, cur) && cur.id == DTI_EQUAL) {
						if (get_token(tokens, i++, cur) && cur.id == DTI_NUMBER) {
							var.id = (uint8)cur.value_int;
							if (cur.value_int >= 1 && cur.value_int <= 0xFF) {
								if (def.used_ids.find(var.id) != def.used_ids.end()) {
									printf("Syntax error at %s:%d, variable ID %u has already been used in this definition.\n", cur.fn.c_str(), cur.line, var.id);
									exit(1);
								}
								def.used_ids.insert(var.id);
								if (get_token(tokens, i++, cur) && cur.id == DTI_EOS) {
									def.variables.push_back(var);
									var.Reset();
								} else {
									printf("Syntax error at %s:%d, expected ; after variable ID.\n", cur.fn.c_str(), cur.line);
									exit(1);
								}
							} else {
								printf("Syntax error at %s:%d, variable ID must be from 1 to 255.\n", cur.fn.c_str(), cur.line);
								exit(1);
							}
						} else {
							printf("Syntax error at %s:%d, expected number after =.\n", cur.fn.c_str(), cur.line);
							exit(1);
						}
					} else {
						printf("Syntax error at %s:%d, expected = after variable name.\n", cur.fn.c_str(), cur.line);
						exit(1);
					}
				} else {
					printf("Syntax error at %s:%d, invalid name '%s'. Allowed characters are: %s\n", cur.fn.c_str(), cur.line, var.name.c_str(), ALLOWED_NAME_CHARS);
					exit(1);
				}
			} else {
				printf("Syntax error at %s:%d, expected variable name.\n", cur.fn.c_str(), cur.line);
				exit(1);
			}
		} else {
			printf("Syntax error at %s:%d, expected definition variable entry or close parenthesis.\n", cur.fn.c_str(), cur.line);
			exit(1);
		}
	}
	if (definitions.size() == 0) {
		printf("This file has no definitions!\n");
		exit(1);
	}

	FILE * fp = fopen(fn_h.c_str(), "wb");
	if (fp == NULL) {
		printf("Error opening %s for output: %s\n", fn_h.c_str(), strerror(errno));
		exit(1);
	}
	output_header(fp);
	fprintf(fp, "#ifndef __DSL_SERIALIZE_H__\n#include <drift/dslcore.h>\n#include <drift/serialize.h>\n#endif\n\n");
	for (auto x : definitions) {
		fprintf(fp, "class %s : public DSL_Serializable_DSD {\n", x.name.c_str());
		fprintf(fp, "public:\n");
		for (auto v : x.variables) {
			if (v.type == DTI_TYPE_STRING) {
				if (v.vector) {
					fprintf(fp, "\tvector<string> %s;\n", v.name.c_str());
				} else {
					fprintf(fp, "\tstring %s;\n", v.name.c_str());
				}
			} else if (v.type == DTI_TYPE_CUSTOM) {
				if (v.vector) {
					fprintf(fp, "\tvector<%s> %s;\n", v.custom_type.c_str(), v.name.c_str());
				} else {
					fprintf(fp, "\t%s %s;\n", v.custom_type.c_str(), v.name.c_str());
				}
			} else if (v.type == DTI_TYPE_CHAR) {
				fprintf(fp, "\tchar %s[" U64FMT "] = {0};\n", v.name.c_str(), v.size);
			} else if (v.type == DTI_TYPE_BYTES) {
				fprintf(fp, "\tuint8 %s[" U64FMT "] = {0};\n", v.name.c_str(), v.size);
			} else if (IsNumericType(v.type) && v.type >= DTI_TYPE_INT8) {
				int size = 8 * (1 << (v.type - DTI_TYPE_INT8));
				if (v.vector) {
					fprintf(fp, "\tvector<int%d> %s;\n", size, v.name.c_str());
				} else {
					fprintf(fp, "\tint%d %s = 0;\n", size, v.name.c_str());
				}
			} else if (IsNumericType(v.type)) {
				int size = 8 * (1 << (v.type - DTI_TYPE_UINT8));
				if (v.vector) {
					fprintf(fp, "\tvector<uint%d> %s;\n", size, v.name.c_str());
				} else {
					fprintf(fp, "\tuint%d %s = 0;\n", size, v.name.c_str());
				}
			} else {
				printf("Internal error: we don't know how to handle variable %s :(\n", v.name.c_str());
			}
		}
		//fprintf(fp, "\n\tbool IsValid();\n");
		fprintf(fp, "\n");
		fprintf(fp, "protected:\n");
		fprintf(fp, "\tbool Serialize(DSL_BUFFER * buf, bool deserialize);\n");
		//fprintf(fp, "\tset<uint8> have_ids;\n");
		fprintf(fp, "};\n\n");
	}
	fclose(fp);

	fp = fopen(fn_cpp.c_str(), "wb");
	if (fp == NULL) {
		printf("Error opening %s for output: %s\n", fn_cpp.c_str(), strerror(errno));
		exit(1);
	}
	output_header(fp);
	fprintf(fp, "#include \"%s\"\n\n", nopath(fn_h.c_str()));
	for (auto x : definitions) {
		fprintf(fp, "bool %s::Serialize(DSL_BUFFER * serbuf, bool deserialize) {\n", x.name.c_str());
		fprintf(fp, "\tuint8 id;\n");
		fprintf(fp, "\tif (deserialize) {\n");		
		fprintf(fp, "\t\tDSL_BUFFER * buf = serbuf;\n");
		fprintf(fp, "\t\tuint32 len;\n");
		fprintf(fp, "\t\twhile (buf->len > 1) {\n");
		fprintf(fp, "\t\t\tid = buf->udata[0];\n");
		fprintf(fp, "\t\t\tbuffer_remove_front(buf, 1);\n");
		fprintf(fp, "\t\t\tser(&len);\n");
		fprintf(fp, "\t\t\tif (id == 0) { break; }\n");
		for (auto v : x.variables) {
			if (v.type == DTI_TYPE_STRING) {
				if (v.vector) {
					fprintf(fp, "\t\t\telse if (id == %d) { servstr(%s); }\n", v.id, v.name.c_str());
				} else {
					fprintf(fp, "\t\t\telse if (id == %d) { ser(&%s); }\n", v.id, v.name.c_str());
				}
			} else if (v.type == DTI_TYPE_CHAR) {
				fprintf(fp, "\t\t\telse if (id == %d) { memset(%s, 0, sizeof(%s)); ser2(%s, sizeof(%s)); }\n", v.id, v.name.c_str(), v.name.c_str(), v.name.c_str(), v.name.c_str());
			} else if (v.type == DTI_TYPE_BYTES) {
				fprintf(fp, "\t\t\telse if (id == %d) { ser2(%s, sizeof(%s)); }\n", v.id, v.name.c_str(), v.name.c_str());
			} else if (IsNumericType(v.type) && v.type >= DTI_TYPE_INT8) {
				int size = 8 * (1 << (v.type - DTI_TYPE_INT8));
				if (v.vector) {
					fprintf(fp, "\t\t\telse if (id == %d) { serv(%s, int%d); }\n", v.id, v.name.c_str(), size);
				} else {
					fprintf(fp, "\t\t\telse if (id == %d) { ser(&%s); }\n", v.id, v.name.c_str());
				}
			} else if (IsNumericType(v.type)) {
				int size = 8 * (1 << (v.type - DTI_TYPE_UINT8));
				if (v.vector) {
					fprintf(fp, "\t\t\telse if (id == %d) { serv(%s, uint%d); }\n", v.id, v.name.c_str(), size);
				} else {
					fprintf(fp, "\t\t\telse if (id == %d) { ser(&%s); }\n", v.id, v.name.c_str());
				}
			} else if (v.type == DTI_TYPE_CUSTOM) {
				if (v.vector) {
					fprintf(fp, "\t\t\telse if (id == %d) { serv2(%s, %s); }\n", v.id, v.name.c_str(), v.custom_type.c_str());
				} else {
					fprintf(fp, "\t\t\telse if (id == %d) { string tmp; ser(&tmp); if (!%s.FromSerialized(tmp)) { return false; } }\n", v.id, v.name.c_str());
				}
			} else {
				printf("Internal error: we don't know how to handle variable %s :(\n", v.name.c_str());
			}
		}
		fprintf(fp, "\t\t\telse {\n");
		fprintf(fp, "\t\t\t\tbuffer_remove_front(buf, len);\n");//skip this entry, we it's from a newer version of the definition we don't know about
		fprintf(fp, "\t\t\t}\n");
		fprintf(fp, "\t\t}\n");
		fprintf(fp, "\t\treturn true;\n");

		fprintf(fp, "\t} else {\n");

		fprintf(fp, "\t\tDSL_BUFFER * buf = dsl_new(DSL_BUFFER);\n");
		fprintf(fp, "\t\tbuffer_init(buf);\n");
		for (auto v : x.variables) {
			if (v.type == DTI_TYPE_STRING) {
				if (v.vector) {
					fprintf(fp, "\t\tif (%s.size()) { servstr(%s); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf); }\n", v.name.c_str(), v.name.c_str(), v.id);
				} else {
					fprintf(fp, "\t\tif (%s.length()) { ser(&%s); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf); }\n", v.name.c_str(), v.name.c_str(), v.id);
				}
			} else if (v.type == DTI_TYPE_CHAR) {
				fprintf(fp, "\t\tif (strlen(%s)) { ser2(%s, strlen(%s)); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf); }\n", v.name.c_str(), v.name.c_str(), v.name.c_str(), v.id);
			} else if (v.type == DTI_TYPE_BYTES) {
				fprintf(fp, "\t\tser2(%s, sizeof(%s)); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf);\n", v.name.c_str(), v.name.c_str(), v.id);
			} else if (IsNumericType(v.type) && v.type >= DTI_TYPE_INT8) {
				int size = 8 * (1 << (v.type - DTI_TYPE_INT8));
				if (v.vector) {
					fprintf(fp, "\t\tif (%s.size()) { serv(%s, int%d); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf); }\n", v.name.c_str(), v.name.c_str(), size, v.id);
				} else {
					fprintf(fp, "\t\tif (%s) { ser(&%s); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf); }\n", v.name.c_str(), v.name.c_str(), v.id);
				}
			} else if (IsNumericType(v.type)) {
				int size = 8 * (1 << (v.type - DTI_TYPE_UINT8));
				if (v.vector) {
					fprintf(fp, "\t\tif (%s.size()) { serv(%s, uint%d); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf); }\n", v.name.c_str(), v.name.c_str(), size, v.id);
				} else {
					fprintf(fp, "\t\tif (%s) { ser(&%s); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf); }\n", v.name.c_str(), v.name.c_str(), v.id);
				}
			} else if (v.type == DTI_TYPE_CUSTOM) {
				if (v.vector) {
					fprintf(fp, "\t\tserv2(%s, %s); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf);\n", v.name.c_str(), v.custom_type.c_str(), v.id);
				} else {
					fprintf(fp, "\t\t{ string tmp = %s.GetSerialized(); ser(&tmp); dsd_entries[%u] = buffer_as_string(buf); buffer_clear(buf); }\n", v.name.c_str(), v.id);
				}
			} else {
				printf("Internal error: we don't know how to handle variable %s :(\n", v.name.c_str());
			}
		}
		fprintf(fp, "\t\tbuffer_free(buf);\n");
		fprintf(fp, "\t\tdsl_free(buf);\n");
		fprintf(fp, "\t\tbool ret = serializeEntries(serbuf);\n");
		fprintf(fp, "\t\tdsd_entries.clear();\n");
		fprintf(fp, "\t\treturn ret;\n");
		fprintf(fp, "\t}\n");
		fprintf(fp, "}\n");
		fprintf(fp, "\n");
	}
	/*
	class SerializationTest : public DSL_Serializable {
	public:
		string test1;
		char test2[64] = { 0 };
		int test3 = 0;

	protected:
		bool Serialize(DSL_BUFFER * buf, bool deserialize) {
			ser(&test1);
			ser2(test2, sizeof(test2));
			ser(&test3);
			return true;
		}
	};
	*/
	fclose(fp);

	printf("Compilation completed successfully!\n");
	exit(0);
}
