//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#ifndef __DSL_H__
#define __DSL_H__

#include <drift/dslcore.h>

#include <drift/os_version.h>
#include <drift/ini-reader.h>
#include <drift/WhereIs.h>
#include <drift/DynamicLinking.h>
#include <drift/base64.h>
#include <drift/GenLib.h>
#include <drift/hash.h>
#include <drift/hmac.h>

#if !defined(NO_CPLUSPLUS)
#include <drift/rwops.h>
#include <drift/config.h>
#include <drift/config2.h>
#include <drift/Mutex.h>
#include <drift/Buffer.h>
#include <drift/sockets3.h>
#include <drift/download.h>
#include <drift/Threading.h>
#include <drift/SyncedInt.h>
#include <drift/Directory.h>

#include <drift/DB_Common.h>
#if defined(ENABLE_MYSQL)
#include <drift/DB_MySQL.h>
#endif // ENABLE_MYSQL
#if defined(ENABLE_SQLITE)
#include <drift/DB_SQLite.h>
#endif // ENABLE_MYSQL
#endif // NO_CPLUSPLUS

#if defined(WIN32) && !defined(NO_CPLUSPLUS) // Win32-only files
#include <drift/win32/poll.h>
#include <drift/win32/getopt.h>
#include <drift/win32/registry.h>
#endif

#if defined(ENABLE_OPENSSL)
#include <drift/openssl.h>
#endif

#if defined(ENABLE_GNUTLS)
#include <drift/gnutls.h>
#endif

#if defined(ENABLE_SODIUM)
#include <drift/sodium.h>
#endif

#ifdef MEMLEAK
#include <drift/memleak.h>
#endif

#endif // __DSL_H__
