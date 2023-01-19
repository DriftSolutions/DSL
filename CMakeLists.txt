cmake_minimum_required (VERSION 3.1)

project (DSL)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckCXXCompilerFlag)

option(ENABLE_STATIC "Enable static library outputs" ON)
option(ENABLE_SHARED "Enable DLL outputs" ON)
IF(ENABLE_SHARED)
message(STATUS "Building shared libraries")
ENDIF()
IF(ENABLE_STATIC)
message(STATUS "Building static libraries")
ENDIF()

add_definitions(-DDSL_DEFAULT_MUTEX_TIMEOUT=1800000 -DENABLE_ZLIB -DDSL_NO_COMPAT)

include_directories(.)
IF (WIN32)

SET(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
SET(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FS")
include_directories(c:/memleak/drift c:/memleak/include)
SET(LIBS zlib-static)

IF("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
message(STATUS "Mode: 64-bit")
link_directories(c:/memleak/lib64)
ELSE()
message(STATUS "Mode: 32-bit")
link_directories(c:/memleak/lib)
ENDIF()

ELSE(WIN32)

INCLUDE(${CMAKE_ROOT}/Modules/FindPkgConfig.cmake)

include_directories(/usr/include/drift /usr/include/mysql /usr/local/include/mysql /usr/local/include)
link_directories(/usr/lib/mysql /usr/local/lib/mysql /usr/local/lib)

IF("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
message(STATUS "Mode: 64-bit")
link_directories(/usr/lib64/mysql /usr/local/lib64/mysql /usr/lib/x86_64-linux-gnu)
ELSE()
message(STATUS "Mode: 32-bit")
link_directories(/usr/lib/i386-linux-gnu)
ENDIF()

SET(LIBS dl pthread)

pkg_search_module(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})
add_definitions(-DENABLE_ZLIB)
SET(LIBS ${LIBS} ${ZLIB_LIBRARIES})

FIND_PACKAGE(OpenSSL REQUIRED)
SET(LIBS ${LIBS} ${OPENSSL_LIBRARIES})
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_PATH})

FIND_PACKAGE(GnuTLS REQUIRED)
SET(LIBS ${LIBS} ${GNUTLS_LIBRARIES})
INCLUDE_DIRECTORIES(${GNUTLS_INCLUDE_PATH})

pkg_search_module(LIBCURL REQUIRED libcurl)
include_directories(${LIBCURL_INCLUDE_DIRS})
link_directories(${LIBCURL_LIBRARY_DIRS})
SET(LIBS ${LIBS} ${LIBCURL_LIBRARIES})

pkg_search_module(PHYSFS REQUIRED libphysfs physfs libphysicsfs physicsfs)
include_directories(${PHYSFS_INCLUDE_DIRS})
link_directories(${PHYSFS_LIBRARY_DIRS})
SET(LIBS ${LIBS} ${PHYSFS_LIBRARIES})

pkg_search_module(SQLITE REQUIRED sqlite3 sqlite)
include_directories(${SQLITE_INCLUDE_DIRS})
link_directories(${SQLITE_LIBRARY_DIRS})
SET(LIBS ${LIBS} ${SQLITE_LIBRARIES})

pkg_search_module(LIBEVENT REQUIRED libevent_pthreads event_pthreads libevent2_pthreads event2_pthreads)
include_directories(${LIBEVENT_INCLUDE_DIRS})
link_directories(${LIBEVENT_LIBRARY_DIRS})
SET(LIBS ${LIBS} ${LIBEVENT_LIBRARIES})

pkg_search_module(MYSQL REQUIRED mysqlclient_r mysqlclient)
include_directories(${MYSQL_INCLUDE_DIRS})
link_directories(${MYSQL_LIBRARY_DIRS})
SET(LIBS ${LIBS} ${MYSQL_LIBRARIES})

pkg_search_module(SODIUM REQUIRED sodium libsodium)
include_directories(${SODIUM_INCLUDE_DIRS})
link_directories(${SODIUM_LIBRARY_DIRS})
SET(LIBS ${LIBS} ${SODIUM_LIBRARIES})

add_definitions(-fPIC -pthread)
add_definitions(-Wall -D_REENTRANT -D_THREAD_SAFE -D_POSIX_C_SOURCE=200112 -D_FORTIFY_SOURCE=2 -D_FILE_OFFSET_BITS=64)

function (OptTest option)
	check_cxx_compiler_flag(${option} OPTCHK)
	IF(OPTCHK)
		message(STATUS "GCC has ${option}")
		add_definitions(${option})
	ELSE(OPTCHK)
		message(STATUS "GCC does not have ${option}")
	ENDIF(OPTCHK)
	UNSET(OPTCHK CACHE)
endfunction(OptTest)
function (OptTest2 option def)
	check_cxx_compiler_flag(${option} OPTCHK)
	IF(OPTCHK)
		message(STATUS "GCC has ${option}")
		add_definitions(${option} ${def})
	ELSE(OPTCHK)
		message(STATUS "GCC does not have ${option}")
	ENDIF(OPTCHK)
	UNSET(OPTCHK CACHE)
endfunction(OptTest2)
OptTest2(-fvisibility=hidden -DHAVE_VIS)
OptTest2(-mrdrnd -DGCC_RDRAND)
OptTest(-fstack-protector-all)
#OptTest(-Wstack-protector)
OptTest(-Wno-pragmas)
OptTest(-Wno-write-strings)
OptTest(-Wno-format-contains-nul)
OptTest(-Werror=format-security)
OptTest(-Werror=format-extra-args)
#CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
#CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
#if(COMPILER_SUPPORTS_CXX11)
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#elseif(COMPILER_SUPPORTS_CXX0X)
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
#	OptTest(-fpermissive)
#else()
#	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
#endif()

ENDIF(WIN32)

SET(BASENAME dsl)

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")

#enable debugging
message(STATUS "Debug build")
SET(LIBPOSTFIX _d)
add_definitions(-DDEBUG -D_DEBUG)
IF (WIN32)
ELSE (WIN32)
#add_definitions(-D_GLIBCXX_DEBUG)
add_definitions(-g -ggdb)
ENDIF (WIN32)

ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")

#release mode
message(STATUS "Release build")
SET(LIBPOSTFIX )
IF (WIN32)
ELSE (WIN32)
add_definitions(-DNDEBUG)
add_definitions(-O3)
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -s")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -s")
ENDIF (WIN32)

ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

SET(CORESTATIC ${BASENAME}-core-static${LIBPOSTFIX})
SET(COREDLL ${BASENAME}-core${LIBPOSTFIX})

message(STATUS "Building ${COREDLL}")

AUX_SOURCE_DIRECTORY(src SRCFILES1)
AUX_SOURCE_DIRECTORY(src/win32 SRCFILES2)
AUX_SOURCE_DIRECTORY(src/algo SRCFILES3)
AUX_SOURCE_DIRECTORY(drift SRCHEADERS)

# Build DSL core static lib
IF(ENABLE_STATIC AND WIN32)
add_library(${CORESTATIC} STATIC ${SRCFILES1} ${SRCFILES2} ${SRCFILES3} ${SRCHEADERS})
target_compile_definitions(${CORESTATIC} PRIVATE DSL_EXPORTS DSL_STATIC)
set_target_properties(${CORESTATIC} PROPERTIES COMPILE_PDB_NAME "${CORESTATIC}")
ENDIF()

# Build DSL core DLL
IF(ENABLE_SHARED)
add_library(${COREDLL} SHARED ${SRCFILES1} ${SRCFILES2} ${SRCFILES3} ${SRCHEADERS})
target_compile_definitions(${COREDLL} PRIVATE DSL_EXPORTS DSL_DLL)
set_target_properties(${COREDLL} PROPERTIES COMPILE_PDB_NAME "${COREDLL}")
TARGET_LINK_LIBRARIES(${COREDLL} ${LIBS})
ENDIF()

# Build dsl-mysql
AUX_SOURCE_DIRECTORY(src/mysql SRCFILESMYSQL)

IF(ENABLE_STATIC AND WIN32)
add_library(${BASENAME}-mysql-static${LIBPOSTFIX} STATIC ${SRCFILESMYSQL})
target_compile_definitions(${BASENAME}-mysql-static${LIBPOSTFIX} PRIVATE DSL_STATIC DSL_MYSQL_EXPORTS ENABLE_MYSQL)
add_dependencies(${BASENAME}-mysql-static${LIBPOSTFIX} ${CORESTATIC})
set_target_properties(${BASENAME}-mysql-static${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-mysql-static${LIBPOSTFIX}")
ENDIF()

IF(ENABLE_SHARED)
add_library(${BASENAME}-mysql${LIBPOSTFIX} SHARED ${SRCFILESMYSQL})
TARGET_LINK_LIBRARIES(${BASENAME}-mysql${LIBPOSTFIX} ${LIBS})
target_compile_definitions(${BASENAME}-mysql${LIBPOSTFIX} PRIVATE DSL_DLL DSL_MYSQL_EXPORTS ENABLE_MYSQL)
set_target_properties(${BASENAME}-mysql${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-mysql${LIBPOSTFIX}")
add_dependencies(${BASENAME}-mysql${LIBPOSTFIX} ${COREDLL})
ENDIF()

# End dsl-mysql

# Build dsl-sqlite
AUX_SOURCE_DIRECTORY(src/sqlite SRCFILESSQLITE)
#SET(LIBS ${LIBS} sqlite3${LIBPOSTFIX}.lib)

IF(ENABLE_STATIC AND WIN32)
add_library(${BASENAME}-sqlite-static${LIBPOSTFIX} STATIC ${SRCFILESSQLITE})
target_compile_definitions(${BASENAME}-sqlite-static${LIBPOSTFIX} PRIVATE DSL_STATIC DSL_SQLITE_EXPORTS ENABLE_SQLITE)
add_dependencies(${BASENAME}-sqlite-static${LIBPOSTFIX} ${CORESTATIC})
set_target_properties(${BASENAME}-sqlite-static${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-sqlite-static${LIBPOSTFIX}")
ENDIF()

IF(ENABLE_SHARED)
add_library(${BASENAME}-sqlite${LIBPOSTFIX} SHARED ${SRCFILESSQLITE})
TARGET_LINK_LIBRARIES(${BASENAME}-sqlite${LIBPOSTFIX} ${LIBS})
target_compile_definitions(${BASENAME}-sqlite${LIBPOSTFIX} PRIVATE DSL_DLL DSL_SQLITE_EXPORTS ENABLE_SQLITE)
set_target_properties(${BASENAME}-sqlite${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-sqlite${LIBPOSTFIX}")
add_dependencies(${BASENAME}-sqlite${LIBPOSTFIX} ${COREDLL})
ENDIF()

# End dsl-sqlite

# Build dsl-curl
AUX_SOURCE_DIRECTORY(src/curl SRCFILESCURL)
#SET(LIBS ${LIBS} libcurl${LIBPOSTFIX}.lib)

IF(ENABLE_STATIC AND WIN32)
add_library(${BASENAME}-curl-static${LIBPOSTFIX} STATIC ${SRCFILESCURL})
target_compile_definitions(${BASENAME}-curl-static${LIBPOSTFIX} PRIVATE DSL_STATIC DSL_CURL_EXPORTS ENABLE_CURL)
add_dependencies(${BASENAME}-curl-static${LIBPOSTFIX} ${CORESTATIC})
set_target_properties(${BASENAME}-curl-static${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-curl-static${LIBPOSTFIX}")
ENDIF()

IF(ENABLE_SHARED)
add_library(${BASENAME}-curl${LIBPOSTFIX} SHARED ${SRCFILESCURL})
TARGET_LINK_LIBRARIES(${BASENAME}-curl${LIBPOSTFIX} ${LIBS})
target_compile_definitions(${BASENAME}-curl${LIBPOSTFIX} PRIVATE DSL_DLL DSL_CURL_EXPORTS ENABLE_CURL)
set_target_properties(${BASENAME}-curl${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-curl${LIBPOSTFIX}")
add_dependencies(${BASENAME}-curl${LIBPOSTFIX} ${COREDLL})
ENDIF()

# End dsl-curl

# Build dsl-physfs
AUX_SOURCE_DIRECTORY(src/physfs SRCFILESPHYSFS)

IF(ENABLE_STATIC AND WIN32)
add_library(${BASENAME}-physfs-static${LIBPOSTFIX} STATIC ${SRCFILESPHYSFS})
target_compile_definitions(${BASENAME}-physfs-static${LIBPOSTFIX} PRIVATE DSL_STATIC DSL_PHYSFS_EXPORTS ENABLE_PHYSFS)
add_dependencies(${BASENAME}-physfs-static${LIBPOSTFIX} ${CORESTATIC})
set_target_properties(${BASENAME}-physfs-static${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-physfs-static${LIBPOSTFIX}")
ENDIF()

IF(ENABLE_SHARED)
add_library(${BASENAME}-physfs${LIBPOSTFIX} SHARED ${SRCFILESPHYSFS})
TARGET_LINK_LIBRARIES(${BASENAME}-physfs${LIBPOSTFIX} ${LIBS})
target_compile_definitions(${BASENAME}-physfs${LIBPOSTFIX} PRIVATE DSL_DLL DSL_PHYSFS_EXPORTS ENABLE_PHYSFS)
set_target_properties(${BASENAME}-physfs${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-physfs${LIBPOSTFIX}")
add_dependencies(${BASENAME}-physfs${LIBPOSTFIX} ${COREDLL})
ENDIF()

# End dsl-physfs

# Build dsl-sodium
AUX_SOURCE_DIRECTORY(src/sodium SRCFILESSODIUM)

IF(ENABLE_STATIC AND WIN32)
add_library(${BASENAME}-sodium-static${LIBPOSTFIX} STATIC ${SRCFILESSODIUM})
target_compile_definitions(${BASENAME}-sodium-static${LIBPOSTFIX} PRIVATE DSL_STATIC DSL_SODIUM_EXPORTS ENABLE_SODIUM)
add_dependencies(${BASENAME}-sodium-static${LIBPOSTFIX} ${CORESTATIC})
set_target_properties(${BASENAME}-sodium-static${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-sodium-static${LIBPOSTFIX}")
ENDIF()

IF(ENABLE_SHARED)
add_library(${BASENAME}-sodium${LIBPOSTFIX} SHARED ${SRCFILESSODIUM})
TARGET_LINK_LIBRARIES(${BASENAME}-sodium${LIBPOSTFIX} ${LIBS})
target_compile_definitions(${BASENAME}-sodium${LIBPOSTFIX} PRIVATE DSL_DLL DSL_SODIUM_EXPORTS ENABLE_SODIUM)
set_target_properties(${BASENAME}-sodium${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-sodium${LIBPOSTFIX}")
add_dependencies(${BASENAME}-sodium${LIBPOSTFIX} ${COREDLL})
ENDIF()

# End dsl-sodium

# Build dsl-openssl
AUX_SOURCE_DIRECTORY(src/openssl SRCFILESOPENSSL)

IF(ENABLE_STATIC AND WIN32)
add_library(${BASENAME}-openssl-static${LIBPOSTFIX} STATIC ${SRCFILESOPENSSL})
target_compile_definitions(${BASENAME}-openssl-static${LIBPOSTFIX} PRIVATE DSL_STATIC DSL_OPENSSL_EXPORTS ENABLE_OPENSSL)
add_dependencies(${BASENAME}-openssl-static${LIBPOSTFIX} ${CORESTATIC})
set_target_properties(${BASENAME}-openssl-static${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-openssl-static${LIBPOSTFIX}")
ENDIF()

IF(ENABLE_SHARED)
add_library(${BASENAME}-openssl${LIBPOSTFIX} SHARED ${SRCFILESOPENSSL})
TARGET_LINK_LIBRARIES(${BASENAME}-openssl${LIBPOSTFIX} ${LIBS})
target_compile_definitions(${BASENAME}-openssl${LIBPOSTFIX} PRIVATE DSL_DLL DSL_OPENSSL_EXPORTS ENABLE_OPENSSL)
set_target_properties(${BASENAME}-openssl${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-openssl${LIBPOSTFIX}")
add_dependencies(${BASENAME}-openssl${LIBPOSTFIX} ${COREDLL})
ENDIF()

# End dsl-openssl

# Build dsl-gnutls
AUX_SOURCE_DIRECTORY(src/gnutls SRCFILESGNUTLS)

IF(ENABLE_STATIC AND WIN32)
add_library(${BASENAME}-gnutls-static${LIBPOSTFIX} STATIC ${SRCFILESGNUTLS})
target_compile_definitions(${BASENAME}-gnutls-static${LIBPOSTFIX} PRIVATE DSL_STATIC DSL_GNUTLS_EXPORTS ENABLE_GNUTLS)
add_dependencies(${BASENAME}-gnutls-static${LIBPOSTFIX} ${CORESTATIC})
set_target_properties(${BASENAME}-gnutls-static${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-gnutls-static${LIBPOSTFIX}")
ENDIF()

IF(ENABLE_SHARED)
add_library(${BASENAME}-gnutls${LIBPOSTFIX} SHARED ${SRCFILESGNUTLS})
TARGET_LINK_LIBRARIES(${BASENAME}-gnutls${LIBPOSTFIX} ${LIBS})
target_compile_definitions(${BASENAME}-gnutls${LIBPOSTFIX} PRIVATE DSL_DLL DSL_GNUTLS_EXPORTS ENABLE_GNUTLS)
set_target_properties(${BASENAME}-gnutls${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-gnutls${LIBPOSTFIX}")
add_dependencies(${BASENAME}-gnutls${LIBPOSTFIX} ${COREDLL})
ENDIF()

# End dsl-gnutls

# Build dsl-libevent
AUX_SOURCE_DIRECTORY(src/libevent SRCFILESLIBEVENT)

IF(ENABLE_STATIC AND WIN32)
add_library(${BASENAME}-libevent-static${LIBPOSTFIX} STATIC ${SRCFILESLIBEVENT})
target_compile_definitions(${BASENAME}-libevent-static${LIBPOSTFIX} PRIVATE DSL_STATIC DSL_LIBEVENT_EXPORTS ENABLE_LIBEVENT)
add_dependencies(${BASENAME}-libevent-static${LIBPOSTFIX} ${CORESTATIC})
set_target_properties(${BASENAME}-libevent-static${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-libevent-static${LIBPOSTFIX}")
ENDIF()

IF(ENABLE_SHARED)
add_library(${BASENAME}-libevent${LIBPOSTFIX} SHARED ${SRCFILESLIBEVENT})
TARGET_LINK_LIBRARIES(${BASENAME}-libevent${LIBPOSTFIX} ${LIBS})
target_compile_definitions(${BASENAME}-libevent${LIBPOSTFIX} PRIVATE DSL_DLL DSL_LIBEVENT_EXPORTS ENABLE_LIBEVENT)
set_target_properties(${BASENAME}-libevent${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "${BASENAME}-libevent${LIBPOSTFIX}")
add_dependencies(${BASENAME}-libevent${LIBPOSTFIX} ${COREDLL})
ENDIF()

# End dsl-libevent

IF(ENABLE_STATIC AND NOT WIN32)
	add_library(${BASENAME}-static${LIBPOSTFIX} STATIC ${SRCFILES1} ${SRCFILES2} ${SRCFILES3} ${SRCHEADERS} ${SRCFILESCURL} ${SRCFILESMYSQL} ${SRCFILESSQLITE} ${SRCFILESPHYSFS} ${SRCFILESSODIUM} ${SRCFILESOPENSSL} ${SRCFILESGNUTLS} ${SRCFILESLIBEVENT})
	target_compile_definitions(${BASENAME}-static${LIBPOSTFIX} PRIVATE DSL_STATIC ENABLE_PHYSFS ENABLE_MYSQL ENABLE_SQLITE ENABLE_CURL ENABLE_SODIUM ENABLE_OPENSSL ENABLE_GNUTLS ENABLE_LIBEVENT)
	target_link_libraries(${BASENAME}-static${LIBPOSTFIX} ${LIBS})
ENDIF()

# Build dsd_compiler

AUX_SOURCE_DIRECTORY(dsd_compiler DSDFILES)
add_executable(dsd_compiler${LIBPOSTFIX} ${DSDFILES})
IF(ENABLE_STATIC)
	target_compile_definitions(dsd_compiler${LIBPOSTFIX} PRIVATE DSL_STATIC)
	IF(WIN32)
		target_link_libraries(dsd_compiler${LIBPOSTFIX} ${CORESTATIC} ${LIBS})
		set_target_properties(dsd_compiler${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "dsd_compiler${LIBPOSTFIX}")
		add_dependencies(dsd_compiler${LIBPOSTFIX} ${CORESTATIC})
	ELSE(WIN32)
		target_link_libraries(dsd_compiler${LIBPOSTFIX} ${BASENAME}-static${LIBPOSTFIX} ${LIBS})
		set_target_properties(dsd_compiler${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "dsd_compiler${LIBPOSTFIX}")
		add_dependencies(dsd_compiler${LIBPOSTFIX} ${BASENAME}-static${LIBPOSTFIX})
	ENDIF()
ELSE(ENABLE_STATIC)
	target_compile_definitions(dsd_compiler${LIBPOSTFIX} PRIVATE DSL_DLL)
	target_link_libraries(dsd_compiler${LIBPOSTFIX} ${COREDLL})
	set_target_properties(dsd_compiler${LIBPOSTFIX} PROPERTIES COMPILE_PDB_NAME "dsd_compiler${LIBPOSTFIX}")
	add_dependencies(dsd_compiler${LIBPOSTFIX} ${COREDLL})
ENDIF()

# End dsd_compiler

# Build tests

file(GLOB files "tests/*.cpp")
foreach(file ${files})
	get_filename_component(FN ${file} NAME_WE)
	IF(ENABLE_STATIC)
		add_executable(test-${FN}-static ${file})
		target_compile_definitions(test-${FN}-static PRIVATE DSL_STATIC ENABLE_PHYSFS ENABLE_MYSQL ENABLE_SQLITE ENABLE_CURL ENABLE_SODIUM ENABLE_OPENSSL ENABLE_GNUTLS ENABLE_LIBEVENT)
		IF(WIN32)
			target_link_libraries(test-${FN}-static ${BASENAME}-physfs-static${LIBPOSTFIX} ${BASENAME}-mysql-static${LIBPOSTFIX} ${BASENAME}-sqlite-static${LIBPOSTFIX} ${BASENAME}-curl-static${LIBPOSTFIX} ${BASENAME}-sodium-static${LIBPOSTFIX} ${BASENAME}-openssl-static${LIBPOSTFIX} ${BASENAME}-gnutls-static${LIBPOSTFIX} ${BASENAME}-libevent-static${LIBPOSTFIX} ${CORESTATIC} ${LIBS})
			add_dependencies(test-${FN}-static ${CORESTATIC} ${BASENAME}-physfs-static${LIBPOSTFIX} ${BASENAME}-mysql-static${LIBPOSTFIX} ${BASENAME}-sqlite-static${LIBPOSTFIX} ${BASENAME}-curl-static${LIBPOSTFIX} ${BASENAME}-sodium-static${LIBPOSTFIX} ${BASENAME}-openssl-static${LIBPOSTFIX} ${BASENAME}-gnutls-static${LIBPOSTFIX} ${BASENAME}-libevent-static${LIBPOSTFIX})
			set_target_properties(test-${FN}-static PROPERTIES COMPILE_PDB_NAME "test-${FN}-static")
		ELSE(WIN32)
			target_link_libraries(test-${FN}-static ${BASENAME}-static${LIBPOSTFIX} ${LIBS})
			add_dependencies(test-${FN}-static ${BASENAME}-static${LIBPOSTFIX})			
		ENDIF()
	ENDIF()
	IF(ENABLE_SHARED)
		add_executable(test-${FN} ${file})
		target_compile_definitions(test-${FN} PRIVATE DSL_DLL ENABLE_PHYSFS ENABLE_MYSQL ENABLE_SQLITE ENABLE_CURL ENABLE_SODIUM ENABLE_OPENSSL ENABLE_GNUTLS ENABLE_LIBEVENT)
		target_link_libraries(test-${FN} ${COREDLL} ${BASENAME}-physfs${LIBPOSTFIX} ${BASENAME}-mysql${LIBPOSTFIX} ${BASENAME}-sqlite${LIBPOSTFIX} ${BASENAME}-curl${LIBPOSTFIX} ${BASENAME}-sodium${LIBPOSTFIX} ${BASENAME}-openssl${LIBPOSTFIX} ${BASENAME}-gnutls${LIBPOSTFIX} ${BASENAME}-libevent${LIBPOSTFIX})
		set_target_properties(test-${FN} PROPERTIES COMPILE_PDB_NAME "test-${FN}")
		add_dependencies(test-${FN} ${COREDLL} ${BASENAME}-physfs${LIBPOSTFIX} ${BASENAME}-mysql${LIBPOSTFIX} ${BASENAME}-sqlite${LIBPOSTFIX} ${BASENAME}-curl${LIBPOSTFIX} ${BASENAME}-sodium${LIBPOSTFIX} ${BASENAME}-openssl${LIBPOSTFIX} ${BASENAME}-gnutls${LIBPOSTFIX} ${BASENAME}-libevent${LIBPOSTFIX})
	ENDIF()
endforeach()

# End tests
