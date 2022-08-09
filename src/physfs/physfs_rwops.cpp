//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#if defined(ENABLE_PHYSFS)

#include <drift/dslcore.h>
#include <drift/rwops.h>
#include <physfs.h>

DSL_LIBRARY_FUNCTIONS dsl_physfs_funcs = {
	false,
	DSL_OPTION_PHYSFS,
	NULL,
	NULL,
	NULL,
};
DSL_Library_Registerer dsl_physfs_autoreg(dsl_physfs_funcs);

int64 physfs_read(void * buf, int64 size, DSL_FILE * fp) {
	return PHYSFS_readBytes((PHYSFS_file *)fp->handle, buf, size);
}

int64 physfs_write(void * buf, int64 size, DSL_FILE * fp) {
	return PHYSFS_writeBytes((PHYSFS_file *)fp->handle, buf, size);
}

bool physfs_seek(DSL_FILE * fp, int64 pos, int mode) {
	switch(mode) {
		case SEEK_SET:
			return (PHYSFS_seek((PHYSFS_file *)fp->handle,pos) != 0) ? true : false;
			break;
		case SEEK_CUR:
			return (PHYSFS_seek((PHYSFS_file *)fp->handle,fp->tell(fp)+pos) != 0) ? true : false;
			break;
		case SEEK_END:{
				PHYSFS_sint64 len = PHYSFS_fileLength((PHYSFS_file *)fp->handle);
				return (PHYSFS_seek((PHYSFS_file *)fp->handle,len+pos) != 0) ? true : false;
			}
			break;
		default:
			return false;
			break;
	}
}


int64 physfs_tell(DSL_FILE * fp) {
	return PHYSFS_tell((PHYSFS_file *)fp->handle);
}

bool physfs_flush(DSL_FILE * fp) {
	return (PHYSFS_flush((PHYSFS_file *)fp->handle) != 0);
}

bool physfs_eof(DSL_FILE * fp) {
	return (PHYSFS_eof((PHYSFS_file *)fp->handle) != 0);
}

void physfs_close(DSL_FILE * fp) {
	PHYSFS_file * o_fp = (PHYSFS_file *)fp->handle;
	bool autoclose = true;
	if (fp->p_extra) {
		TP_RWOPT * opt = (TP_RWOPT *)fp->p_extra;
		autoclose = opt->autoclose;
		delete opt;
	}
	if (autoclose) {
		PHYSFS_close(o_fp);
	}
	delete fp;
}

DSL_FILE * DSL_CC RW_OpenPhysFS(const char * fn, const char * mode) {
	PHYSFS_file * fp = NULL;
	if (strstr(mode, "a")) {
		fp = PHYSFS_openAppend(fn);
	} else if (strstr(mode, "w")) {
		fp = PHYSFS_openWrite(fn);
	} else if (strstr(mode, "r")) {
		fp = PHYSFS_openRead(fn);
	}
	if (!fp) { return NULL; }

	DSL_FILE * ret = new DSL_FILE;
	memset(ret,0,sizeof(DSL_FILE));

	ret->handle = fp;
	ret->read = physfs_read;
	ret->write = physfs_write;
	ret->seek = physfs_seek;
	ret->tell = physfs_tell;
	ret->close = physfs_close;
	ret->flush = physfs_flush;
	ret->eof = physfs_eof;

	return ret;
};

DSL_FILE * DSL_CC RW_ConvertPhysFS(PHYSFS_file * fp, bool autoclose) {
	DSL_FILE * ret = new DSL_FILE;
	memset(ret,0,sizeof(DSL_FILE));

	ret->handle = fp;
	ret->read = physfs_read;
	ret->write = physfs_write;
	ret->seek = physfs_seek;
	ret->tell = physfs_tell;
	ret->close = physfs_close;
	ret->flush = physfs_flush;
	ret->eof = physfs_eof;

	if (!autoclose) {
		TP_RWOPT * opt = new TP_RWOPT;
		memset(opt,0,sizeof(TP_RWOPT));
		opt->autoclose = autoclose;
		ret->p_extra = opt;
	}

	return ret;
};

#endif // ENABLE_PHYSFS
