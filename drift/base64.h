//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_BASE64_H__
#define __DSL_BASE64_H__

/**
 * \defgroup base64 Base64 Encoding & Decoding
 */

/** \addtogroup base64
 * @{
 */

#ifndef DOXYGEN_SKIP
#ifdef DSL_OLD_BASE64
#define encodestring(out, in, count) base64_encode(in, count, out)
#define decodestring(out, in, count) base64_decode(in, count, out)
#endif
#endif

#define base64_encode_buffer_size(x) ((4*(x/3))+1)

/**
 * Encodes binary data to a base64 string. outBuffer should be at least base64_encode_buffer_size(count) bytes long.
 * @return The length of the encoded string.
 */
DSL_API int DSL_CC base64_encode(const void *inBuffer, size_t count, char *outBuffer);
/**
 * Decodes a base64 string to binary data.
 * @return The length of the decoded data.
 */
DSL_API int DSL_CC base64_decode(const char *inBuffer, size_t count, void *outBuffer);

/**@}*/

#endif // __DSL_BASE64_H__
