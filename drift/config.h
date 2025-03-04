//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __UNIVERSAL_CONFIG_H__
#define __UNIVERSAL_CONFIG_H__

#include <drift/ds_value.h>

#pragma pack(1)
struct DS_VALUE {
	DS_VALUE_TYPE type;
	union {
		char * pString;
		void * pBinary;
		union {
			uint32 uLong;
			int32 lLong;
			double lFloat;
		};
	};
};

struct DS_CONFIG_VALUE {
	char name[64];
	DS_VALUE value;
	DS_CONFIG_VALUE *Prev,*Next;
};

struct DS_CONFIG_SECTION {
	char name[64];
	DS_CONFIG_VALUE * values;
	DS_CONFIG_SECTION * sections; // sub-sections
	DS_CONFIG_SECTION *Prev,*Next,*Parent;
};
#pragma pack()

typedef std::vector<DS_CONFIG_SECTION *> scanStackType;

class DSL_API_CLASS Universal_Config {
	private:
		DS_CONFIG_SECTION *fSection, *lSection;

		scanStackType scanStack;
		//DS_CONFIG_SECTION *LastScan[20];

		void FreeSection(DS_CONFIG_SECTION * Scan);
		void WriteSection(FILE * fp, DS_CONFIG_SECTION * sec, int level);
		void WriteBinarySection(FILE * fp, DS_CONFIG_SECTION * sec);
		void PrintSection(DS_CONFIG_SECTION * Scan, int level);
		bool IsLong(const char * text);
		bool IsFloat(const char * text);
		DS_CONFIG_SECTION * GetSectionFromString(const char * sec, bool create=false);

		DS_CONFIG_SECTION * PopScan();
		void PushScan(DS_CONFIG_SECTION * section);

	public:
		Universal_Config();
		~Universal_Config();

		bool LoadConfig(const char * filename, DS_CONFIG_SECTION * Scan=NULL);
		bool LoadConfig(FILE * fp, const char * filename, DS_CONFIG_SECTION * Scan=NULL);
		bool LoadBinaryConfig(const char * filename);
		bool LoadBinaryConfig(FILE * fp);

		bool WriteConfig(const char * filename, DS_CONFIG_SECTION * Start=NULL, bool Single=false);
		bool WriteConfig(FILE * fp, DS_CONFIG_SECTION * Start=NULL, bool Single=false);
		bool WriteBinaryConfig(const char * filename, DS_CONFIG_SECTION * Start=NULL, bool Single=false);
		bool WriteBinaryConfig(FILE * fp, DS_CONFIG_SECTION * Start=NULL, bool Single=false);

		void FreeConfig();

		DS_CONFIG_SECTION * GetSection(DS_CONFIG_SECTION * parent, const char * name);
		DS_VALUE * GetSectionValue(DS_CONFIG_SECTION * sec, const char * name);

		DS_VALUE * GetValue(const char * sec, const char * name);
		const char * GetValueString(const char * sec, const char * name);
		int32 GetValueLong(const char * sec, const char * name);
		double GetValueFloat(const char * sec, const char * name);
		void SetValue(const char * sec, const char * name, DS_VALUE * val);
		void SetValueString(const char * sec, const char * name, const char * val);
		void SetValueLong(const char * sec, const char * name, int32 val);
		void SetValueFloat(const char * sec, const char * name, double val);

		DS_VALUE * SetSectionValue(DS_CONFIG_SECTION * sec, const char * name, DS_VALUE * val);

		// advanced commands
		DS_CONFIG_SECTION *FindOrAddSection(DS_CONFIG_SECTION * parent, const char * name);

		void ClearSection(DS_CONFIG_SECTION * Scan);
		void PrintConfigTree();
};

#endif // __UNIVERSAL_CONFIG_H__
