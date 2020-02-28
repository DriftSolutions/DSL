//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

DSL_API const char * DSL_CC GetOSVersion();
DSL_API bool DSL_CC Is64Bit();

struct CPUInformation {
	bool bRDTSC : 1,	// Is RDTSC supported?
		 bCMOV  : 1,  // Is CMOV supported?
		 bFCMOV : 1,  // Is FCMOV supported?
		 bSSE	  : 1,	// Is SSE supported?
		 bSSE2  : 1,	// Is SSE2 Supported?
		 b3DNow : 1,	// Is 3DNow! Supported?
		 bMMX   : 1,	// Is MMX supported?
		 bHT	  : 1;	// Is HyperThreading supported?

	uint8 nLogicalProcessors;		// Number op logical processors.
	uint8 nPhysicalProcessors;		// Number of physical processors

	int64 Speed;					// In cycles per second.

	char ProcessorID[32];			// Processor vendor Identification.
};

//DSL_API CPUInformation DSL_CC GetCPUInformation();
