#include <drift/dslcore.h>
#include <drift/ds_value.h>

DS_Value::DS_Value() {
	Int = 0;
}

DS_Value::~DS_Value() {
	Reset();
}

void DS_Value::Reset() {
	Type = DS_TYPE_UNKNOWN;
	memset(&Int, 0, sizeof(Int));
	sString.clear();
}

bool DS_Value::AsBool() const {
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
int64 DS_Value::AsInt() const {
	if (Type == DS_TYPE_STRING || Type == DS_TYPE_BINARY) {
		return atoi64(sString.c_str());
	} else if (Type == DS_TYPE_FLOAT) {
		return (int64)Float;
	} else if (Type == DS_TYPE_INT || Type == DS_TYPE_BOOL) {
		return Int;
	}
	return 0;
}
double DS_Value::AsFloat() const {
	if (Type == DS_TYPE_STRING || Type == DS_TYPE_BINARY) {
		return atof(sString.c_str());
	} else if (Type == DS_TYPE_FLOAT) {
		return Float;
	} else if (Type == DS_TYPE_INT || Type == DS_TYPE_BOOL) {
		return (double)Int;
	}
	return 0;
}

string DS_Value::AsString() const {
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

const char * DS_Value::c_str() const {
	if (Type == DS_TYPE_STRING || Type == DS_TYPE_BINARY) {
		return sString.c_str();
	} else {
		return AsString().c_str();
	}
}

void DS_Value::SetValue(const DS_Value& val) {
	Reset();
	Type = val.Type;
	if (val.Type == DS_TYPE_STRING || val.Type == DS_TYPE_BINARY) {
		sString = val.sString;
	} else {
		Int = val.Int;
	}
}
void DS_Value::SetValue(int64 val) {
	Reset();
	Type = DS_TYPE_INT;
	Int = val;
}
void DS_Value::SetValue(double val) {
	Reset();
	Type = DS_TYPE_FLOAT;
	Float = val;
}
void DS_Value::SetValue(const char * val) {
	Reset();
	Type = DS_TYPE_STRING;
	sString = val;
}
void DS_Value::SetValue(const string& val) {
	Reset();
	Type = DS_TYPE_STRING;
	sString = val;
}
void DS_Value::SetValue(const uint8_t * val, size_t len) {
	Reset();
	Type = DS_TYPE_BINARY;
	sString.assign((const char *)val, len);
}
void DS_Value::SetValue(bool val) {
	Reset();
	Type = DS_TYPE_BOOL;
	Int = val;
}

void DS_Value::ParseString(const char * value) {
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

bool DS_Value::isBool(const char * buf, bool * val) {
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

bool DS_Value::isInt(const char * buf) {
	if (buf == NULL || buf[0] == 0) { return false; } // null/empty string

	int64 l = atoi64(buf);
	if (l == 0 && strcmp(buf, "0")) {
		return false;
	}
	if (l == INT64_MAX && stricmp(buf, "9223372036854775807")) {
		return false;
	}
	if (l == INT64_MIN && stricmp(buf, "-9223372036854775807") && stricmp(buf, "-9223372036854775808")) {
		return false;
	}

	//int periodCnt=0;
	for (int i = 0; buf[i] != 0; i++) {
		if (buf[i] == '-' && i == 0 && buf[1] != 0) {
			continue;
		}
		if (buf[i] < 48 || buf[i] > 57) { // not a number
			return false;
		}
	}

	return true;
}

bool DS_Value::isFloat(const char * buf) {
	if (buf == NULL || buf[0] == 0) { return false; } // null/empty string

	double l = atof(buf);
	if (l == 0.0 && strcmp(buf, "0.0") && strcmp(buf, "0.00")) {
		return false;
	}
	if (l == HUGE_VAL || l == -HUGE_VAL) {
		return false;
	}

	int periodCnt = 0;
	for (int i = 0; buf[i] != 0; i++) {
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

	return (periodCnt == 1) ? true : false;
}


/*
void DS_Value::Print(FILE * fp) {
}
*/
