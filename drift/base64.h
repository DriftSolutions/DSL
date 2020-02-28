//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.0.0                    |
|                  Copyright 2010-2020 Drift Solutions                  |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#ifndef __DSL_BASE64_H__
#define __DSL_BASE64_H__

#ifdef DSL_OLD_BASE64
#define encodestring(out, in, count) base64_encode(in, count, out)
#define decodestring(out, in, count) base64_decode(in, count, out)
#endif

DSL_API int DSL_CC base64_encode(const void *inBuffer, size_t count, char *outBuffer);
DSL_API int DSL_CC base64_decode(const char *inBuffer, size_t count, void *outBuffer);

#endif // __DSL_BASE64_H__
