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
#include <drift/os_version.h>

#ifdef WIN32
const char * DSL_CC GetOSVersion() {
	if (Is64Bit()) {
		return "Windows (64-bit)";
	} else {
		return "Windows (32-bit)";
	}
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process=NULL;

bool DSL_CC Is64Bit() {
#ifdef WIN64
	return true;//we're good until Windows x128 is released
#else
	if (fnIsWow64Process == NULL) {
		fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32"),"IsWow64Process");
	}
	if (fnIsWow64Process == NULL) { return false; }

    BOOL bIsWow64 = FALSE;
    if (fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
		return bIsWow64;
	}
	return false;
#endif
}

#else

static char Get_OS_Version_Buf[1024];
const char * DSL_CC GetOSVersion() {
	struct utsname name;
	memset(&name, 0, sizeof(name));
	if (uname(&name) >= 0) {
		sprintf(Get_OS_Version_Buf, "%s %s %s on %s", name.sysname, name.version, name.release, name.machine);
		return Get_OS_Version_Buf;
	} else {
		#ifdef POSIX_C_SOURCE
			return "POSIX-compliant OS";
		#else
			return "Non-Windows OS";
		#endif
	}
}

bool DSL_CC Is64Bit() {
#if defined(__x86_64__)
	return true;
#else
	return false;
#endif
}

#ifdef DSL_HAVE_CPUID
DSL_API void DSL_CC linux_cpuid(int cpuInfo[4], int function_id) {
	__cpuid(function_id, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
}
#endif

#endif

#ifdef DSL_HAVE_CPUID
const InstructionSet::InstructionSet_Internal InstructionSet::CPU_Rep;
#endif
