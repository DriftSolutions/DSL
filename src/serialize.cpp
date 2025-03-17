#include <drift/dslcore.h>
#include <drift/GenLib.h>
#include <drift/serialize.h>
#include <assert.h>

bool DSL_Serializable::_serialize_int(DSL_BUFFER * buf, void * val, uint8_t size, bool deserialize) {
	assert(size == 1 || size == 2 || size == 4 || size == 8);
	return deserialize ? dsl_deserialize_int(buf, val, size) : dsl_serialize_int(buf, val, size);
}
bool DSL_Serializable::serialize_var(DSL_BUFFER * buf, string * val, bool deserialize) {
	return deserialize ? dsl_deserialize_string(buf, val) : dsl_serialize_string(buf, val);
}
bool DSL_Serializable::serialize_var(DSL_BUFFER * buf, char * data, size_t bufSize, bool deserialize) {
	return deserialize ? dsl_deserialize_varchar(buf, (uint8_t *)data, bufSize) : dsl_serialize_varchar(buf, (const uint8_t *)data, strlen(data));
}
bool DSL_Serializable::serialize_var(DSL_BUFFER * buf, uint8_t * data, size_t lSize, bool deserialize) {
	return deserialize ? dsl_deserialize_fixed(buf, data, lSize) : dsl_serialize_fixed(buf, data, lSize);
}

size_t _max_size_of_varint(uint8_t size) {
	assert(size == 1 || size == 2 || size == 4 || size == 8);
	unsigned int x = (size * 8) / 7;
	if ((size * 8) % 7 != 0) {
		x++;
	}
	return x;
}

bool DSL_Serializable::GetSerialized(DSL_BUFFER * buf) {
	return Serialize(buf, false);
}

bool DSL_Serializable::FromSerialized(DSL_BUFFER * buf) {
	return Serialize(buf, true);
}

string DSL_Serializable::GetSerialized() {
	DSL_BUFFER buf;
	buffer_init(&buf);
	string ret;
	if (GetSerialized(&buf)) {
		ret.assign(buf.data, buf.len);
	}
	buffer_free(&buf);
	return ret;
}
bool DSL_Serializable::FromSerialized(const string& str) {
	DSL_BUFFER buf;
	buffer_init(&buf);
	buffer_set(&buf, str.data(), str.length());
	bool ret = FromSerialized(&buf);
	buffer_free(&buf);
	return ret;
}

#ifdef ENABLE_ZLIB

bool DSL_Serializable::GetCompressed(DSL_BUFFER * buf, int compression_level) {
	bool ret = false;
	if (Serialize(buf, false)) {
		DSL_BUFFER buf2;
		buffer_init(&buf2);
		uLongf dlen = compressBound(buf->len);
		if ((sizeof(uLongf) == 4 || dlen <= UINT32_MAX)) {
			buffer_resize(&buf2, dlen);
			if (compress2(buf2.udata, &dlen, buf->udata, buf->len, compression_level) == Z_OK) {
				uint32_t serlen = buf->len;
				buffer_clear(buf);
				if (dsl_serialize_int(buf, &serlen, sizeof(serlen))) {
					if (buffer_append(buf, buf2.data, dlen)) {
						ret = true;
					}
				}
			}
		}
		buffer_free(&buf2);
	}
	return ret;
}

bool DSL_Serializable::FromCompressed(DSL_BUFFER * buf) {
	bool ret = false;
	DSL_BUFFER buf2;
	buffer_init(&buf2);
	uint32_t serlen = 0;
	if (dsl_deserialize_int(buf, &serlen, sizeof(serlen))) {
		uLongf dlen = serlen;
		buffer_resize(&buf2, serlen);
		int n = 0;
		if ((n = uncompress(buf2.udata, &dlen, buf->udata, buf->len)) == Z_OK && dlen == serlen) {
			buffer_clear(buf);
			if (buffer_append(buf, buf2.data, dlen)) {
				if (Serialize(buf, true)) {
					ret = true;
				}
			}
		}
	}
	buffer_free(&buf2);
	return ret;
}

string DSL_Serializable::GetCompressed(int compression_level) {
	DSL_BUFFER buf;
	buffer_init(&buf);
	string ret;
	if (GetCompressed(&buf, compression_level)) {
		ret.assign(buf.data, buf.len);
	}
	buffer_free(&buf);
	return ret;
}
bool DSL_Serializable::FromCompressed(const string& str) {
	DSL_BUFFER buf;
	buffer_init(&buf);
	buffer_set(&buf, str.data(), str.length());
	bool ret = FromCompressed(&buf);
	buffer_free(&buf);
	return ret;
}

#endif

bool DSL_Serializable_DSD::serializeEntries(DSL_BUFFER * buf) {
	uint32 len;
	for (auto& x : dsd_entries) {
		buffer_append_int<uint8>(buf, x.first);
#if SIZE_MAX > UINT32_MAX
		if (x.second.length() > UINT32_MAX) { return false; }
#endif
		len = x.second.length();
		if (!serialize_var(buf, &len, false)) { return false; }
		if (!buffer_append(buf, x.second.c_str(), x.second.length())) { return false; }
	}
	return true;
}

struct DSL_SERIALIZE_INTINMEM {
	union {
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
	};
};

bool DSL_CC dsl_serialize_int(DSL_BUFFER * buf, const void * pVal, size_t lSize) {
	assert(lSize == 1 || lSize == 2 || lSize == 4 || lSize == 8);
	uint64_t tmp = 0;
	const DSL_SERIALIZE_INTINMEM * intinmem = (const DSL_SERIALIZE_INTINMEM *)pVal;
	switch (lSize) {
		case 1:
			tmp = intinmem->u8;
			break;
		case 2:
			tmp = intinmem->u16;
			break;
		case 4:
			tmp = intinmem->u32;
			break;
		case 8:
			tmp = intinmem->u64;
			break;
		default:
			return false;
			break;
	}
	uint8_t data;
	bool first = true;
	while (tmp || first) {
		first = false;
		data = (tmp & 0x7F);
		tmp >>= 7;
		if (tmp) {
			//more data to follow
			data |= 0x80;
		}
		buffer_append_int<uint8_t>(buf, data);
	}
	return true;
}

bool DSL_CC dsl_deserialize_int(DSL_BUFFER * buf, void * pVal, size_t lSize) {
	assert(lSize == 1 || lSize == 2 || lSize == 4 || lSize == 8);
	size_t used = 0;
	uint8_t * data = buf->udata;
	size_t maxlen = _max_size_of_varint(lSize);
	while (1) {
		if (buf->len < ++used) {
			return false;
		}
		if (used > maxlen) {
			/* worst case scenario should be 10 bytes */
			return false;
		}
		if (!(*data & 0x80)) {
			break;
		}
		data++;
	}
	uint64_t tmp = 0;
	while (data >= buf->udata) {
		tmp <<= 7;
		tmp |= (*data & 0x7F);
		data--;
	}
	buffer_remove_front(buf, used);

	DSL_SERIALIZE_INTINMEM * intinmem = (DSL_SERIALIZE_INTINMEM *)pVal;
	switch (lSize) {
		case 1:
			if (tmp > UINT8_MAX) {
				return false;
			}
			intinmem->u8 = tmp;
			break;
		case 2:
			if (tmp > UINT16_MAX) {
				return false;
			}
			intinmem->u16 = tmp;
			break;
		case 4:
			if (tmp > UINT32_MAX) {
				return false;
			}
			intinmem->u32 = tmp;
			break;
		case 8:
			intinmem->u64 = tmp;
			break;
		default:
			return false;
			break;
	}

	return true;
}

bool DSL_CC dsl_serialize_string(DSL_BUFFER * buf, const string * val) {
	return dsl_serialize_varchar(buf, (const uint8_t *)val->data(), val->length());
}
bool DSL_CC dsl_deserialize_string(DSL_BUFFER * buf, string * val) {
	uint32 lSize = 0;
	if (!dsl_deserialize_int(buf, &lSize, sizeof(lSize))) {
		return false;
	}
	if (buf->len < lSize) {
		return false;
	}
	if (lSize > 0) {
		val->assign(buf->data, lSize);
		buffer_remove_front(buf, lSize);
	} else {
		val->clear();
	}
	return true;
}

bool DSL_CC dsl_serialize_varchar(DSL_BUFFER * buf, const uint8_t * val, uint32_t lSize) {
	if (!dsl_serialize_int(buf, &lSize, sizeof(lSize))) {
		return false;
	}
	return buffer_append(buf, (const char *)val, lSize);
}
bool DSL_CC dsl_deserialize_varchar(DSL_BUFFER * buf, uint8_t * val, uint32_t lSize) {
	uint32_t len = 0;
	if (!dsl_deserialize_int(buf, &len, sizeof(len))) {
		return false;
	}
	if (len > lSize || buf->len < len) {
		return false;
	}
	if (len > 0) {
		memcpy(val, buf->udata, len);
		buffer_remove_front(buf, len);
	}
	return true;
}

bool DSL_CC dsl_serialize_fixed(DSL_BUFFER * buf, const uint8_t * val, uint32_t lSize) {
	return buffer_append(buf, (const char *)val, lSize);
}
bool DSL_CC dsl_deserialize_fixed(DSL_BUFFER * buf, uint8_t * val, uint32_t lSize) {
	if (buf->len < lSize) {
		return false;
	}
	if (lSize > 0) {
		memcpy(val, buf->udata, lSize);
		buffer_remove_front(buf, lSize);
	}
	return true;
}

bool DSL_CC dsl_serialize_vector_string(DSL_BUFFER * buf, vector<string>& vec, bool deserialize) {
	if (deserialize) {
		uint32 num;
		if (!dsl_deserialize_int(buf, &num, sizeof(num))) {	return false; }
		vec.clear();
		string tmp;
		for (uint32 i = 0; i < num; i++) {
			if (!dsl_deserialize_string(buf, &tmp)) { return false; }
			vec.push_back(tmp);
		}
	} else {
		uint32 num = (uint32)vec.size();
		if (!dsl_serialize_int(buf, &num, sizeof(num))) { return false; }
		for (auto& x : vec) {
			if (!dsl_serialize_string(buf, &x)) { return false; }
		}
	}
	return true;
}

bool DSL_CC dsl_serialize_set_string(DSL_BUFFER * buf, set<string>& vec, bool deserialize) {
	if (deserialize) {
		uint32 num;
		if (!dsl_deserialize_int(buf, &num, sizeof(num))) { return false; }
		vec.clear();
		string tmp;
		for (uint32 i = 0; i < num; i++) {
			if (!dsl_deserialize_string(buf, &tmp)) { return false; }
			vec.insert(tmp);
		}
	} else {
		uint32 num = (uint32)vec.size();
		if (!dsl_serialize_int(buf, &num, sizeof(num))) { return false; }
		for (auto& x : vec) {
			if (!dsl_serialize_string(buf, &x)) { return false; }
		}
	}
	return true;
}
