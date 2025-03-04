#pragma once

enum DS_VALUE_TYPE {
	DS_TYPE_UNKNOWN,
	DS_TYPE_BOOL,
	DS_TYPE_INT,
	DS_TYPE_FLOAT,
	DS_TYPE_STRING,
	DS_TYPE_BINARY,
	DS_TYPE_LONG = DS_TYPE_INT, // for legacy code
};

class DSL_API_CLASS DS_Value {
	friend class ConfigSection;
private:
	bool isBool(const char * buf, bool * val = NULL);
	bool isInt(const char * text);
	bool isFloat(const char * text);

protected:
	string sString;
	union {
		int64 Int;
		double Float;
	};

public:
	DS_Value();
	virtual ~DS_Value();

	DS_VALUE_TYPE Type = DS_TYPE_UNKNOWN;

	int64 AsInt() const;
	double AsFloat() const;
	string AsString() const;
	const char * c_str() const;
	bool AsBool() const; /* true if it is an int > 0, double >= 1.00, or the strings "true" or "on" */

	void SetValue(const DS_Value& val);
	void SetValue(int64 val);
	void SetValue(double val);
	void SetValue(const char * val);
	void SetValue(const string& val);
	void SetValue(const uint8_t * val, size_t len);
	void SetValue(bool val);

	void Reset();
	void ParseString(const char * value);

	DS_Value& operator =(const int64 b) {
		SetValue(b);
		return *this;
	}
	DS_Value& operator =(const double b) {
		SetValue(b);
		return *this;
	}
	DS_Value& operator =(const char * b) {
		SetValue(b);
		return *this;
	}
	DS_Value& operator =(const string& b) {
		SetValue(b);
		return *this;
	}
	DS_Value& operator =(const bool b) {
		SetValue(b);
		return *this;
	}
};
