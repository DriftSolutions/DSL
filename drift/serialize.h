//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_SERIALIZE_H__
#define __DSL_SERIALIZE_H__

#include <drift/Buffer.h>

DSL_API_CLASS class DSL_Serializable {
private:
	bool _serialize_int(DSL_BUFFER * buf, void * val, uint8_t size, bool deserialize);
protected:
	bool serialize_var(DSL_BUFFER * buf, uint8_t * val, bool deserialize) { return _serialize_int(buf, val, 1, deserialize); }
	bool serialize_var(DSL_BUFFER * buf, int8_t * val, bool deserialize) { return _serialize_int(buf, val, 1, deserialize); }
	bool serialize_var(DSL_BUFFER * buf, uint16_t * val, bool deserialize) { return _serialize_int(buf, val, 2, deserialize); }
	bool serialize_var(DSL_BUFFER * buf, int16_t * val, bool deserialize) { return _serialize_int(buf, val, 2, deserialize); }
	bool serialize_var(DSL_BUFFER * buf, uint32_t * val, bool deserialize) { return _serialize_int(buf, val, sizeof(uint32_t), deserialize); }
	bool serialize_var(DSL_BUFFER * buf, int32_t * val, bool deserialize) { return _serialize_int(buf, val, sizeof(int32_t), deserialize); }
	bool serialize_var(DSL_BUFFER * buf, uint64_t * val, bool deserialize) { return _serialize_int(buf, val, sizeof(uint64_t), deserialize); }
	bool serialize_var(DSL_BUFFER * buf, int64_t * val, bool deserialize) { return _serialize_int(buf, val, sizeof(int64_t), deserialize); }
	bool serialize_var(DSL_BUFFER * buf, double * val, bool deserialize) { return _serialize_int(buf, val, sizeof(double), deserialize); }
	bool serialize_var(DSL_BUFFER * buf, float * val, bool deserialize) { return _serialize_int(buf, val, sizeof(float), deserialize); }
	bool serialize_var(DSL_BUFFER * buf, string * val, bool deserialize);
	bool serialize_var(DSL_BUFFER * buf, char * data, size_t dataSize, bool deserialize);
	bool serialize_var(DSL_BUFFER * buf, uint8_t * data, size_t lSize, bool deserialize);
	#define ser(x) if (!serialize_var(buf, x, deserialize)) { return false; }
	#define ser2(x,y) if (!serialize_var(buf, x, y, deserialize)) { return false; }
	template <typename T> bool serialize_vector(DSL_BUFFER * buf, vector<T>& vec, bool deserialize) {
		if (deserialize) {
			uint32 num;
			ser(&num);
			T obj;
			vec.clear();
			for (uint32 i = 0; i < num; i++) {
				ser2((uint8 *)&obj, sizeof(T));
				vec.push_back(obj);
			}
		} else {
			uint32 num = vec.size();
			ser(&num);
			for (auto x = vec.begin(); x != vec.end(); x++) {
				ser2((uint8 *)&x->obj, sizeof(T));
			}
		}
	}
	#define serv(x,y) if (!serialize_vector<y>(buf, x, deserialize)) { return false; }

	virtual bool Serialize(DSL_BUFFER * buf, bool deserialize) = 0;
public:
	string GetSerialized();
	bool FromSerialized(const string& str);
	bool GetSerialized(DSL_BUFFER * buf);
	bool FromSerialized(DSL_BUFFER * buf);

#ifdef ENABLE_ZLIB
	string GetCompressed(int compression_level = 5);
	bool FromCompressed(const string& str);
	bool GetCompressed(DSL_BUFFER * buf, int compression_level = 5);
	bool FromCompressed(DSL_BUFFER * buf);
#endif
};

DSL_API bool DSL_CC dsl_serialize_int(DSL_BUFFER * buf, const void * pVal, size_t lSize);
DSL_API bool DSL_CC dsl_deserialize_int(DSL_BUFFER * buf, void * pVal, size_t lSize);
DSL_API bool DSL_CC dsl_serialize_string(DSL_BUFFER * buf, const string * val);
DSL_API bool DSL_CC dsl_deserialize_string(DSL_BUFFER * buf, string * val);
/* This varchar version encodes the buffer with a variable int before the data specifying the size */
DSL_API bool DSL_CC dsl_serialize_varchar(DSL_BUFFER * buf, const uint8_t * val, uint32_t lSize);
DSL_API bool DSL_CC dsl_deserialize_varchar(DSL_BUFFER * buf, uint8_t * val, uint32_t lSize);
/* This fixed version encodes the whole data buffer of size you specify at a fixed size */
DSL_API bool DSL_CC dsl_serialize_fixed(DSL_BUFFER * buf, const uint8_t * val, uint32_t lSize);
DSL_API bool DSL_CC dsl_deserialize_fixed(DSL_BUFFER * buf, uint8_t * val, uint32_t lSize);

#endif // __DSL_SERIALIZE_H__
