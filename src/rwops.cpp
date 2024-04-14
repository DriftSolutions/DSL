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
#include <drift/rwops.h>
#include <drift/GenLib.h>

int64 file_read(void * buf, int64 size, DSL_FILE * fp) {
	return fread(buf,1,size,fp->fp);
}

int64 file_write(void * buf, int64 size, DSL_FILE * fp) {
	return fwrite(buf,1,size,fp->fp);
}

bool file_seek(DSL_FILE * fp, int64 pos, int whence) {
	return (fseek64(fp->fp, pos, whence) == 0);
}

int64 file_tell(DSL_FILE * fp) {
	return ftell64(fp->fp);
}

bool file_flush(DSL_FILE * fp) {
	return (fflush(fp->fp) == 0);
}

bool file_eof(DSL_FILE * fp) {
	return (feof(fp->fp) != 0);
}

void file_close(DSL_FILE * fp) {
	bool do_close = true;
	if (fp->p_extra) {
		TP_RWOPT * opt = (TP_RWOPT *)fp->p_extra;
		do_close = opt->autoclose;
		dsl_free(opt);
	}
	if (do_close) { fclose(fp->fp); }
	dsl_free(fp);
}

DSL_FILE * DSL_CC RW_OpenFile(const char * fn, const char * mode) {
	FILE * fp = fopen(fn, mode);
	if (fp == NULL) { return NULL; }

	DSL_FILE * ret = dsl_znew(DSL_FILE);

	ret->fp = fp;
	ret->read = file_read;
	ret->write = file_write;
	ret->seek = file_seek;
	ret->tell = file_tell;
	ret->flush = file_flush;
	ret->eof = file_eof;
	ret->close = file_close;

	return ret;
};

DSL_FILE * DSL_CC RW_ConvertFile(FILE * fp, bool autoclose) {
	DSL_FILE * ret = dsl_znew(DSL_FILE);

	ret->fp = fp;
	ret->read = file_read;
	ret->write = file_write;
	ret->seek = file_seek;
	ret->tell = file_tell;
	ret->flush = file_flush;
	ret->eof = file_eof;
	ret->close = file_close;

	TP_RWOPT * opt = dsl_znew(TP_RWOPT);
	opt->autoclose = autoclose;
	ret->p_extra = opt;

	return ret;
};

struct TP_MEMHANDLE {
	DSL_FILE _handle;
	uint8 * mem;
	bool bDelete;
	int64 offset;
	int64 size;
};

int64 mem_read(void * buf, int64 size, DSL_FILE * fp) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp;
	if (size + mem->offset > mem->size) {
		size = mem->size - mem->offset;
	}
	if (mem->offset >= mem->size || size <= 0) { return 0; }

	memcpy(buf, mem->mem + mem->offset, size);
	mem->offset += size;

	return size;
}

int64 mem_write(void * buf, int64 size, DSL_FILE * fp) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp;

	if (size + mem->offset > mem->size) {
		size = mem->size - mem->offset;
	}
	if (mem->offset >= mem->size || size <= 0) { return 0; }

	memcpy(mem->mem + mem->offset, buf, size);
	mem->offset += size;
	return size;
}

bool mem_seek(DSL_FILE * fp, int64 pos, int mode) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp;
	switch(mode) {
		case SEEK_SET:
			mem->offset = pos;
			break;
		case SEEK_CUR:
			mem->offset += pos;
			break;
		case SEEK_END:
			mem->offset = mem->size + pos;
			break;
		default:
			return false;
			break;
	}
	if (mem->offset > mem->size) {
		mem->offset = mem->size;
	}
	if (mem->offset < 0) {
		mem->offset = 0;
	}
	return true;
}

void mem_close(DSL_FILE * fp) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp;
	if (mem->bDelete) {
		dsl_free(mem->mem);
	}
	dsl_free(fp);
}

int64 mem_tell(DSL_FILE * fp) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp;
	return mem->offset;
}

bool mem_eof(DSL_FILE * fp) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp;
	return (mem->offset >= mem->size);
}

bool mem_flush(DSL_FILE * fp) {
	return true;
}

DSL_FILE * DSL_CC RW_OpenMemory(int64 size) {
	TP_MEMHANDLE * mem = dsl_znew(TP_MEMHANDLE);
	DSL_FILE * ret = (DSL_FILE *)&mem->_handle;

	mem->mem = (uint8 *)dsl_malloc(size);
	mem->bDelete = true;
	memset(mem->mem,0,size);
	mem->size = size;

	ret->read = mem_read;
	ret->write = mem_write;
	ret->seek = mem_seek;
	ret->tell = mem_tell;
	ret->eof = mem_eof;
	ret->flush = mem_flush;
	ret->close = mem_close;

	return ret;
}

DSL_FILE * DSL_CC RW_ConvertMemory(uint8 * buf, int64 size) {
	TP_MEMHANDLE * mem = dsl_znew(TP_MEMHANDLE);
	DSL_FILE * ret = (DSL_FILE *)&mem->_handle;

	mem->mem = buf;
	mem->size = size;

	ret->read = mem_read;
	ret->write = mem_write;
	ret->seek = mem_seek;
	ret->tell = mem_tell;
	ret->eof = mem_eof;
	ret->flush = mem_flush;
	ret->close = mem_close;

	return ret;
}

struct TP_BUFHANDLE {
	DSL_FILE _handle;
	DSL_BUFFER * buf;
	bool bDelete;
	int64 offset;
};

int64 buf_read(void * buf, int64 size, DSL_FILE * fp) {
	TP_BUFHANDLE * mem = (TP_BUFHANDLE *)fp;
	if (size + mem->offset > mem->buf->len) {
		size = mem->buf->len - mem->offset;
	}
	if (mem->offset >= mem->buf->len || size <= 0) { return 0; }

	memcpy(buf, mem->buf->udata + mem->offset, size);
	mem->offset += size;

	return size;
}

int64 buf_write(void * buf, int64 size, DSL_FILE * fp) {
	TP_BUFHANDLE * mem = (TP_BUFHANDLE *)fp;

	if (size <= 0) { return 0; }

	int64 newoff = mem->offset + size;
	if (newoff > mem->buf->len) {
		buffer_resize(mem->buf, newoff);
	}

	memcpy(mem->buf->udata + mem->offset, buf, size);
	mem->offset = newoff;
	return size;
}

bool buf_seek(DSL_FILE * fp, int64 pos, int mode) {
	TP_BUFHANDLE * mem = (TP_BUFHANDLE *)fp;
	switch (mode) {
		case SEEK_SET:
			mem->offset = pos;
			break;
		case SEEK_CUR:
			mem->offset += pos;
			break;
		case SEEK_END:
			mem->offset = mem->buf->len + pos;
			break;
		default:
			return false;
			break;
	}
	if (mem->offset > mem->buf->len) {
		mem->offset = mem->buf->len;
	}
	if (mem->offset < 0) {
		mem->offset = 0;
	}
	return true;
}

void buf_close(DSL_FILE * fp) {
	TP_BUFHANDLE * mem = (TP_BUFHANDLE *)fp;
	if (mem->bDelete) {
		buffer_free(mem->buf);
		dsl_free(mem->buf);
	}
	dsl_free(fp);
}

int64 buf_tell(DSL_FILE * fp) {
	TP_BUFHANDLE * mem = (TP_BUFHANDLE *)fp;
	return mem->offset;
}

bool buf_eof(DSL_FILE * fp) {
	TP_BUFHANDLE * mem = (TP_BUFHANDLE *)fp;
	return (mem->offset >= mem->buf->len);
}

bool buf_flush(DSL_FILE * fp) {
	return true;
}

DSL_FILE * DSL_CC RW_OpenBuffer(DSL_BUFFER ** pbuf) {
	TP_BUFHANDLE * mem = dsl_znew(TP_BUFHANDLE);
	DSL_FILE * ret = (DSL_FILE *)&mem->_handle;

	DSL_BUFFER * buf = dsl_znew(DSL_BUFFER);
	buffer_init(buf);

	mem->buf = buf;
	if (pbuf != NULL) {
		*pbuf = buf;
	}
	mem->bDelete = true;

	ret->read = buf_read;
	ret->write = buf_write;
	ret->seek = buf_seek;
	ret->tell = buf_tell;
	ret->eof = buf_eof;
	ret->flush = buf_flush;
	ret->close = buf_close;

	return ret;
}

DSL_FILE * DSL_CC RW_ConvertBuffer(DSL_BUFFER * buf, int64 offset) {
	TP_BUFHANDLE * mem = dsl_znew(TP_BUFHANDLE);
	DSL_FILE * ret = (DSL_FILE *)&mem->_handle;

	mem->buf = buf;
	if (offset < 0) {
		mem->offset = buf->len;
	} else {
		mem->offset = offset;
	}

	ret->read = buf_read;
	ret->write = buf_write;
	ret->seek = buf_seek;
	ret->tell = buf_tell;
	ret->eof = buf_eof;
	ret->flush = buf_flush;
	ret->close = buf_close;

	return ret;
}

