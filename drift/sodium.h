//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_SODIUM_H__
#define __DSL_SODIUM_H__

#include <drift/hash.h>
#include <drift/Buffer.h>

#if defined(DSL_DLL) && defined(WIN32)
	#if defined(DSL_SODIUM_EXPORTS)
		#define DSL_SODIUM_API extern "C" __declspec(dllexport)
		#define DSL_SODIUM_API_CLASS __declspec(dllexport)
	#else
		#define DSL_SODIUM_API extern "C" __declspec(dllimport)
		#define DSL_SODIUM_API_CLASS __declspec(dllimport)
	#endif
#else
	#define DSL_SODIUM_API DSL_API_VIS
	#define DSL_SODIUM_API_CLASS DSL_API_VIS
#endif

extern bool fSodiumInit;

#include <drift/sodium/hash.h>
#include <drift/sodium/keys_signing.h>
#include <drift/sodium/keys_encryption.h>

#endif // __DSL_SODIUM_H__
