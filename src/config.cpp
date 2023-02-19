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
#include <drift/GenLib.h>
#include <math.h>

Universal_Config::Universal_Config() {
	fSection = NULL; // make sure section list is empty
	lSection = NULL;
	//memset(LastScan,0,sizeof(LastScan));
}

Universal_Config::~Universal_Config() {
	FreeConfig();
}

void Universal_Config::ClearSection(DS_CONFIG_SECTION * Scan) {
	DS_CONFIG_VALUE * ValScan = Scan->values;
	Scan->values = NULL;
	while (ValScan) {
		DS_CONFIG_VALUE * ValDel = ValScan;
		if (ValScan->value.type == DS_TYPE_STRING || ValScan->value.type == DS_TYPE_BINARY) {
			//printf("free(): %s\n", ValScan->value.pString);
			dsl_free(ValScan->value.pBinary);
		}
		ValScan = ValScan->Next;
		dsl_free(ValDel);
	}

	DS_CONFIG_SECTION * Scan2 = Scan->sections;
	Scan->sections = NULL;
	while (Scan2) {
		DS_CONFIG_SECTION * toDel=Scan2;
		Scan2=Scan2->Next;
		FreeSection(toDel);
	}
}

void Universal_Config::FreeSection(DS_CONFIG_SECTION * Scan) {
	ClearSection(Scan);
	dsl_free(Scan);
};

void Universal_Config::FreeConfig() {
	DS_CONFIG_SECTION * Scan = fSection;
	while (Scan) {
		DS_CONFIG_SECTION * toDel=Scan;
		Scan=Scan->Next;
		FreeSection(toDel);
	}
	fSection=NULL;
	lSection=NULL;
}

DS_CONFIG_SECTION * Universal_Config::GetSection(DS_CONFIG_SECTION * parent, const char * name) {
	DS_CONFIG_SECTION * Scan = parent ? parent->sections:fSection;

	while(Scan) {
		if (!stricmp(Scan->name,name)) { return Scan; }
		Scan=Scan->Next;
	}

	return NULL;
}

DS_VALUE * Universal_Config::GetSectionValue(DS_CONFIG_SECTION * sec, const char * name) {
	if (!sec) { return NULL; }

	DS_CONFIG_VALUE * Scan = sec->values;
	while (Scan) {
		if (!stricmp(Scan->name,name)) {
			return &Scan->value;
		}
		Scan=Scan->Next;
	}
	return NULL;
}

DS_VALUE * Universal_Config::SetSectionValue(DS_CONFIG_SECTION * sec, const char * name, DS_VALUE * val) {
	if (!sec) { return NULL; }
	DS_VALUE * eval = GetSectionValue(sec,name);
	if (eval) {
		if (val != eval) {
			if (eval->type == DS_TYPE_STRING || eval->type == DS_TYPE_BINARY) {
				dsl_free(eval->pString);
			}
			memcpy(eval,val,sizeof(DS_VALUE));
			if (eval->type == DS_TYPE_STRING) {
				eval->pString = dsl_strdup(val->pString);
			}
		} else {
			printf("Universal_Config() -> You must never pass a DS_VALUE to SetSectionValue that was returned from GetSectionValue\n");
#if defined(WIN32)
			MessageBoxA(0, "Universal_Config() -> You must never pass a DS_VALUE to SetSectionValue that was returned from GetSectionValue", "Error", 0);
#endif
		}
	} else {
		DS_CONFIG_VALUE * Scan = sec->values;
		if (Scan) {
			while (Scan->Next) {
				Scan=Scan->Next;
			}
			Scan->Next = (DS_CONFIG_VALUE *)dsl_malloc(sizeof(DS_CONFIG_VALUE));
			Scan->Next->Next=NULL;
			Scan->Next->Prev = Scan;
			Scan=Scan->Next;
			strcpy(Scan->name,name);
		} else {
			Scan = (DS_CONFIG_VALUE *)dsl_malloc(sizeof(DS_CONFIG_VALUE));
			Scan->Next=NULL;
			Scan->Prev = NULL;
			sec->values = Scan;
			strcpy(Scan->name,name);
		}

		memcpy(&Scan->value,val,sizeof(DS_VALUE));
		if (Scan->value.type == DS_TYPE_STRING) {
			Scan->value.pString = dsl_strdup(val->pString);
		}
		eval = &Scan->value;
	}

	return eval;
}

bool Universal_Config::IsLong(const char * buf) {
	if (buf[0] == 0) { return false; } // null string

	// these next 2 lines detect numbers that would be out of range of a long
	//if (buf[0] == '-' && strlen(buf) > 11) { return false; }
	//if (strlen(buf) > 10) { return false; }

	int32 l = atoi(buf);
	if (l == 0 && strcmp(buf,"0")) {
		return false;
	}

	if (l == 2147483647 && stricmp(buf, "2147483647")) {
		return false;
	}
	if (l == -2147483647 && stricmp(buf, "-2147483647")) {
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
bool Universal_Config::IsFloat(const char * buf) {
	if (buf[0] == 0) { return false; } // null string

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

DS_CONFIG_SECTION * Universal_Config::FindOrAddSection(DS_CONFIG_SECTION * parent, const char * name) {
	DS_CONFIG_SECTION * ret = GetSection(parent,name);
	if (!ret) {
		ret = (DS_CONFIG_SECTION *)dsl_malloc(sizeof(DS_CONFIG_SECTION));//DS_CONFIG_SECTION;
		memset(ret,0,sizeof(DS_CONFIG_SECTION));

		if (!parent) {
			if (fSection) {
				lSection->Next = ret;
				ret->Prev = lSection;
				lSection = ret;
			} else {
				fSection=ret;
				lSection=ret;
			}
		} else {
			if (parent->sections) {
				DS_CONFIG_SECTION * Scan = parent->sections;
				while (Scan->Next != NULL) {
					Scan = Scan->Next;
				}
				Scan->Next = ret;
				Scan->Next->Prev = Scan;
			} else {
				parent->sections = ret;
			}
		}

		ret->Parent = parent;
		strcpy(ret->name,name);
	}
	return ret;
}

DS_CONFIG_SECTION * Universal_Config::PopScan() {
	DS_CONFIG_SECTION * ret = scanStack.back(); // LastScan[0];
	scanStack.pop_back();
	return ret;
}

void Universal_Config::PushScan(DS_CONFIG_SECTION * section) {
	scanStack.push_back(section);
}

bool Universal_Config::LoadConfig(FILE * fp, const char * fn, DS_CONFIG_SECTION * Scan) {
	if (!fp) { return false; }

	char buf[256];
	//char * p=NULL;

	int line=0;
	bool long_comment=false;

	memset(buf, 0, sizeof(buf));

	while (fgets(buf,sizeof(buf),fp)) {
		line++;
		strtrim(buf," \t\r\n "); // first, trim the string of unwanted chars
		if (strlen(buf) < 2) {
			// the minimum meaningful line would be 2 chars long, specifically };
			continue;
		}

		str_replaceA(buf,sizeof(buf),"\t"," "); // turn tabs into spaces
		while (strstr(buf, "  ")) {
			str_replaceA(buf,sizeof(buf),"  "," "); // turn double-spaces into spaces
		}

//		printf("line: %s\n",buf);

		if (stristr(buf, "#include")) {
			char * p = stristr(buf, "#include");
			p = strchr(p, '\"');
			if (p) {
				p++;
				char *q = strchr(p, '\"');
				if (q) {
					q[0]=0;
					if (LoadConfig(p, Scan)) {
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

		if (buf[0] == '#' || !strncmp(buf,"//",2)) {
			continue;
		}

		if (!strncmp(buf,"/*",2)) {
			long_comment = true;
			continue;
		}

		if (long_comment) {
			if (!strcmp(buf,"*/")) {
				long_comment = false;
			}
			continue;
		}

		char *p = (char *)&buf+(strlen(buf) - 2); // will be " {" if beginning a section
		if (!strcmp(p," {")) { // open a new section
			p[0]=0;
			PushScan(Scan);
			Scan = FindOrAddSection(Scan,buf);
			continue;
		}

		if (!strcmp(buf,"};")) { // close section
			if (Scan != NULL) {
				Scan = PopScan();
			} else {
				printf("ERROR: You have one too many }; near line %d of %s\n", line, fn);
				break;
			}
			continue;
		}

		StrTokenizer * tok = new StrTokenizer(buf,' ');
		unsigned long num = tok->NumTok();
		if (num > 1) {
			DS_VALUE * sVal = (DS_VALUE *)dsl_malloc(sizeof(DS_VALUE));
			memset(sVal, 0, sizeof(DS_VALUE));

			char * name = tok->GetSingleTok(1);
			char * value = tok->GetTok(2,num);
			if (IsFloat(value)) { // number
				sVal->type = DS_TYPE_FLOAT;
				sVal->lFloat = atof(value);
			} else if (IsLong(value)) { // number
				sVal->type = DS_TYPE_INT;
				sVal->lLong = atol(value);
			} else { // string
				sVal->type = DS_TYPE_STRING;
				sVal->pString = dsl_strdup(value);
			}
			if (Scan) {
				SetSectionValue(Scan, name, sVal);
			}
			if (sVal->type == DS_TYPE_STRING) {
				dsl_free(sVal->pString);
			}
			dsl_free(sVal);
			tok->FreeString(name);
			tok->FreeString(value);
			delete tok;
			continue;
		}
		delete tok;

		printf("Unrecognized line at %s:%d -> %s\n",fn,line,buf);
		// some unknown line here
	}

	PushScan(Scan);
	return true;
}

bool Universal_Config::LoadConfig(const char * filename, DS_CONFIG_SECTION * Scan) {
	FILE * fp = fopen(filename, "rb");
	if (!fp) { return false; }
	bool ret = LoadConfig(fp, filename, Scan);
	fclose(fp);
	return ret;
}

void Universal_Config::WriteSection(FILE * fp, DS_CONFIG_SECTION * sec, int level) {
	char * pref = (char *)dsl_malloc(level+1);
	for(int i=0; i<level; i++) { pref[i] = '\t'; }
	pref[level]=0;

	char buf[1024];
	sprintf(buf,"%s%s {\n",pref,sec->name);
	fwrite(buf,strlen(buf),1,fp);

	DS_CONFIG_SECTION * Scan = sec->sections;
	while (Scan) {
		WriteSection(fp,Scan,level+1);
		Scan = Scan->Next;
	}

	DS_CONFIG_VALUE * val = sec->values;
	while (val) {
		switch(val->value.type) {
			case DS_TYPE_INT:
				sprintf(buf,"\t%s%s %d\n",pref,val->name,val->value.lLong);
				fwrite(buf,strlen(buf),1,fp);
				break;
			case DS_TYPE_FLOAT:
				sprintf(buf, "\t%s%s %f\n", pref, val->name, val->value.lFloat);
				fwrite(buf, strlen(buf), 1, fp);
				break;
			case DS_TYPE_STRING:
				sprintf(buf,"\t%s%s %s\n",pref,val->name,val->value.pString);
				fwrite(buf,strlen(buf),1,fp);
				break;
			default:
				break;
		}
		val = val->Next;
	};

	sprintf(buf,"%s};\n",pref);
	fwrite(buf,strlen(buf),1,fp);
	dsl_free(pref);
}

bool Universal_Config::WriteConfig(const char * filename, DS_CONFIG_SECTION * Start, bool Single) {
	FILE * fp = fopen(filename,"wb");
	if (!fp) { return false; }

	DS_CONFIG_SECTION * Scan = Start ? Start:this->fSection;
	while (Scan) {
		WriteSection(fp,Scan,0);
		fwrite("\n",1,1,fp);
		if (!Single) {
			Scan = Scan->Next;
		} else {
			Scan = NULL;
		}
	}

	fclose(fp);
	return true;
}


void Universal_Config::PrintSection(DS_CONFIG_SECTION * Scan, int level) {
	char buf[32]={0};
	for (int i=0; i < level; i++) {
		strcat(buf,"\t");
	}

	printf("%sSection: %s\n",buf,Scan->name);
	strcat(buf,"\t");
	DS_CONFIG_VALUE * ValScan = Scan->values;
	while (ValScan) {
		switch(ValScan->value.type) {
			case DS_TYPE_INT:
				printf("%s[%s] = %d\n",buf,ValScan->name,ValScan->value.lLong);
				break;
			case DS_TYPE_FLOAT:
				printf("%s[%s] = %f\n", buf, ValScan->name, ValScan->value.lFloat);
				break;
			case DS_TYPE_STRING:
				printf("%s[%s] = %s\n",buf,ValScan->name,ValScan->value.pString);
				break;
			default:
				printf("%s[%s] = Binary Data @ %p\n",buf,ValScan->name,ValScan->value.pBinary);
				break;
		}
		ValScan = ValScan->Next;
	}

	DS_CONFIG_SECTION * Scan2 = Scan->sections;
	while (Scan2) {
		PrintSection(Scan2,level+1);
		Scan2=Scan2->Next;
	}
};

void Universal_Config::PrintConfigTree() {
	DS_CONFIG_SECTION * Scan = fSection;
	while (Scan) {
		PrintSection(Scan,0);
		Scan=Scan->Next;
	}
}

#pragma pack(1)
struct UC_HEADER {
	unsigned long magic1;
	unsigned long magic2;
	unsigned char version;
};
#pragma pack()

enum UC_ENTRY_TYPE {
	UC_ENTRY_TYPE_SECTION,
	UC_ENTRY_TYPE_UP,
	UC_ENTRY_TYPE_VALUE,
	UC_ENTRY_TYPE_EOF
};

bool Universal_Config::LoadBinaryConfig(const char * filename) {
	FILE * fp = fopen(filename, "rb");
	if (!fp) { return false; }
	bool ret = LoadBinaryConfig(fp);
	fclose(fp);
	return ret;
}

bool Universal_Config::LoadBinaryConfig(FILE * fp) {
	if (!fp) { return false; }
	UC_HEADER head;
	memset(&head, 0, sizeof(head));
	if (fread(&head, sizeof(head), 1, fp) != 1) {
		printf("Error reading file header!\n");
		return false;
	}
	if (head.magic1 != 0x54465244 || head.magic2 != 0x464E4F43) {
		printf("Invalid magic in Dynamic Config file!\n");
		return false;
	}

	if (head.version > 0) {
		printf("Unknown version in Dynamic Config file!\n");
		return false;
	}

	char buf[1024];
	unsigned char type;

	DS_CONFIG_SECTION * sec = NULL;
	DS_VALUE val;
	bool berror = false;
	while (fread(&type, 1, 1, fp) == 1 && !berror) {
		memset(buf, 0, sizeof(buf));
		switch(type) {
			case UC_ENTRY_TYPE_SECTION:
				if (fread(&type, 1, 1, fp) != 1) {
					return false;
				}
				if (fread(buf, type, 1, fp) != 1) {
					return false;
				}
				PushScan(sec);
				sec = FindOrAddSection(sec, buf);
				//printf("New section: %X (%s)\n", sec, sec ? sec->name : "N/A");
				break;
			case UC_ENTRY_TYPE_UP:
				sec = PopScan();
				//printf("Up to %X", sec);
				break;
			case UC_ENTRY_TYPE_VALUE:
				fread(&type, 1, 1, fp);
				fread(buf, type, 1, fp);
				char * name;
				name = dsl_strdup(buf);
				memset(buf, 0, sizeof(buf));
				fread(&type, 1, 1, fp);
				val.type = (DS_VALUE_TYPE)type;
				switch(type) {
					case DS_TYPE_INT:
						fread(&val.lLong,4,1,fp);
						//printf("Got long for %s: %d\n", name, val.lLong);
						break;
					case DS_TYPE_FLOAT:
						fread(&val.lFloat,sizeof(double),1,fp);
						//printf("Got long for %s: %d\n", name, val.lLong);
						break;
					case DS_TYPE_STRING:
						unsigned short len;
						fread(&len,sizeof(len),1,fp);
						fread(buf,len,1,fp);
						val.pString = buf;
						//printf("Got str for %s: %s\n", name, buf);
						break;
				}
				this->SetSectionValue(sec, name, &val);
				dsl_free(name);
				break;
			case UC_ENTRY_TYPE_EOF:
				fseek(fp, 0, SEEK_END);
				break;
			default:
				printf("ERROR: Unknown entry type at offset 0x%08X\n", (unsigned int)ftell(fp));
				berror = true;
				break;
		}
	}

	return true;
}

void Universal_Config::WriteBinarySection(FILE * fp, DS_CONFIG_SECTION * sec) {

	char buf[1024];
	buf[0] = UC_ENTRY_TYPE_SECTION;
	fwrite(buf,1,1,fp);
	buf[0] = (char)strlen(sec->name);
	fwrite(buf,1,1,fp);
	fwrite(sec->name,buf[0],1,fp);

	DS_CONFIG_SECTION * Scan = sec->sections;
	while (Scan) {
		WriteBinarySection(fp,Scan);
		Scan = Scan->Next;
	}

	DS_CONFIG_VALUE * val = sec->values;
	while (val) {
		buf[0] = UC_ENTRY_TYPE_VALUE;
		fwrite(&buf[0],1,1,fp);

		buf[0] = (char)strlen(val->name);
		fwrite(&buf[0],1,1,fp);
		fwrite(val->name,buf[0],1,fp);

		buf[0] = val->value.type;
		fwrite(&buf[0],1,1,fp);

		switch(val->value.type) {
			case DS_TYPE_INT:
				fwrite(&val->value.lLong,4,1,fp);
				break;
			case DS_TYPE_FLOAT:
				fwrite(&val->value.lFloat,sizeof(double),1,fp);
				break;
			case DS_TYPE_STRING:
				unsigned short len;
				len = (unsigned short)strlen(val->value.pString);
				fwrite(&len,sizeof(len),1,fp);
				fwrite(val->value.pString,len,1,fp);
				break;
			default:
				break;
		}
		val = val->Next;
	};

	buf[0] = UC_ENTRY_TYPE_UP;
	fwrite(buf,1,1,fp);
}

bool Universal_Config::WriteBinaryConfig(const char * filename, DS_CONFIG_SECTION * Start, bool Single) {
	FILE * fp = fopen(filename,"wb");
	if (!fp) { return false; }

	UC_HEADER head;
	head.magic1 = 0x54465244;
	head.magic2 = 0x464E4F43;
	head.version = 0;
	fwrite(&head, sizeof(head), 1, fp);

	DS_CONFIG_SECTION * Scan = Start ? Start:this->fSection;
	while (Scan) {
		WriteBinarySection(fp,Scan);
		if (!Single) {
			Scan = Scan->Next;
		} else {
			Scan = NULL;
		}
	}

	unsigned type = UC_ENTRY_TYPE_EOF;
	fwrite(&type,1,1,fp);
	fclose(fp);
	return true;
}

Universal_Config * NewUniversalConfig() {
	return new Universal_Config;
}
void FreeUniversalConfig(Universal_Config * cfg) {
	delete cfg;
}

DS_CONFIG_SECTION * Universal_Config::GetSectionFromString(const char * sec, bool create) {
	if (*sec == '/') { sec++; }
	StrTokenizer st((char *)sec, '/');
	DS_CONFIG_SECTION * ret = NULL;

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

DS_VALUE * Universal_Config::GetValue(const char * ssec, const char * name) {
	DS_CONFIG_SECTION * sec = GetSectionFromString(ssec);
	if (sec) {
		return GetSectionValue(sec, name);
	}
	return NULL;
}

const char * Universal_Config::GetValueString(const char * sec, const char * name) {
	DS_VALUE * v = GetValue(sec, name);
	if (v && v->type == DS_TYPE_STRING) {
		return v->pString;
	}
	return NULL;
}

int32 Universal_Config::GetValueLong(const char * sec, const char * name) {
	DS_VALUE * v = GetValue(sec, name);
	if (v) {
		if (v->type == DS_TYPE_STRING || v->type == DS_TYPE_BINARY) {
			return atol(v->pString);
		} else if (v->type == DS_TYPE_FLOAT) {
			return (int32)v->lFloat;
		} else {
			return v->lLong;
		}
	}
	return 0;
}
double Universal_Config::GetValueFloat(const char * sec, const char * name) {
	DS_VALUE * v = GetValue(sec, name);
	if (v) {
		if (v->type == DS_TYPE_STRING || v->type == DS_TYPE_BINARY) {
			return atof(v->pString);
		} else if (v->type == DS_TYPE_FLOAT) {
			return v->lFloat;
		} else {
			return v->lLong;
		}
	}
	return 0;
}

void Universal_Config::SetValue(const char * ssec, const char * name, DS_VALUE * val) {
	DS_CONFIG_SECTION * sec = GetSectionFromString(ssec, true);
	SetSectionValue(sec, name, val);
}
void Universal_Config::SetValueString(const char * ssec, const char * name, const char * str) {
	DS_CONFIG_SECTION * sec = GetSectionFromString(ssec, true);
	DS_VALUE val;
	val.type = DS_TYPE_STRING;
	val.pString = (char *)str;
	SetSectionValue(sec, name, &val);
}
void Universal_Config::SetValueLong(const char * ssec, const char * name, int32 lval) {
	DS_CONFIG_SECTION * sec = GetSectionFromString(ssec, true);
	DS_VALUE val;
	val.type = DS_TYPE_INT;
	val.lLong = lval;
	SetSectionValue(sec, name, &val);
}
void Universal_Config::SetValueFloat(const char * ssec, const char * name, double lval) {
	DS_CONFIG_SECTION * sec = GetSectionFromString(ssec, true);
	DS_VALUE val;
	val.type = DS_TYPE_FLOAT;
	val.lFloat = lval;
	SetSectionValue(sec, name, &val);
}
