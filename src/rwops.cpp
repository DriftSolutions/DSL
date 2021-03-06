//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
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
	FILE * o_fp = fp->fp;
	bool do_close = true;
	if (fp->p_extra) {
		TP_RWOPT * opt = (TP_RWOPT *)fp->p_extra;
		do_close = opt->autoclose;
		delete opt;
	}
	delete fp;
	if (do_close) { fclose(o_fp); }
}

DSL_FILE * DSL_CC RW_OpenFile(const char * fn, const char * mode) {
	FILE * fp = fopen(fn,mode);
	if (!fp) { return NULL; }

	DSL_FILE * ret = new DSL_FILE;
	memset(ret, 0, sizeof(DSL_FILE));

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
	DSL_FILE * ret = new DSL_FILE;
	memset(ret, 0, sizeof(DSL_FILE));

	ret->fp = fp;
	ret->read = file_read;
	ret->write = file_write;
	ret->seek = file_seek;
	ret->tell = file_tell;
	ret->flush = file_flush;
	ret->eof = file_eof;
	ret->close = file_close;

	TP_RWOPT * opt = new TP_RWOPT;
	memset(opt,0,sizeof(TP_RWOPT));
	opt->autoclose = autoclose;
	ret->p_extra = opt;

	return ret;
};

struct TP_MEMHANDLE {
	uint8 * mem;
	bool bDelete;
	int64 offset;
	int64 size;
};

int64 mem_read(void * buf, int64 size, DSL_FILE * fp) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp->handle;
	if (size + mem->offset > mem->size) {
		size = mem->size - mem->offset;
	}
	if (mem->offset >= mem->size || size <= 0) { return 0; }

	memcpy(buf, mem->mem + mem->offset, size);
	mem->offset += size;

	return size;
}

int64 mem_write(void * buf, int64 size, DSL_FILE * fp) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp->handle;

	if (size + mem->offset > mem->size) {
		size = mem->size - mem->offset;
	}
	if (mem->offset >= mem->size || size <= 0) { return 0; }

	memcpy(mem->mem + mem->offset, buf, size);
	mem->offset += size;
	return size;
}

bool mem_seek(DSL_FILE * fp, int64 pos, int mode) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp->handle;
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
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp->handle;
	if (mem->bDelete) {
		delete mem->mem;
	}
	delete mem;
	delete fp;
}

int64 mem_tell(DSL_FILE * fp) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp->handle;
	return mem->offset;
}

bool mem_eof(DSL_FILE * fp) {
	TP_MEMHANDLE * mem = (TP_MEMHANDLE *)fp->handle;
	return (mem->offset >= mem->size);
}

bool mem_flush(DSL_FILE * fp) {
	return true;
}

DSL_FILE * DSL_CC RW_OpenMemory(int64 size) {
	DSL_FILE * ret = new DSL_FILE;
	memset(ret,0,sizeof(DSL_FILE));
	TP_MEMHANDLE * mem = new TP_MEMHANDLE;
	memset(mem,0,sizeof(TP_MEMHANDLE));

	mem->mem = new uint8[size];
	mem->bDelete = true;
	memset(mem->mem,0,size);
	mem->size = size;

	ret->handle = mem;
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
	DSL_FILE * ret = new DSL_FILE;
	memset(ret,0,sizeof(DSL_FILE));
	TP_MEMHANDLE * mem = new TP_MEMHANDLE;
	memset(mem,0,sizeof(TP_MEMHANDLE));

	mem->mem = buf;
	mem->size = size;

	ret->handle = mem;
	ret->read = mem_read;
	ret->write = mem_write;
	ret->seek = mem_seek;
	ret->tell = mem_tell;
	ret->eof = mem_eof;
	ret->flush = mem_flush;
	ret->close = mem_close;

	return ret;
}

