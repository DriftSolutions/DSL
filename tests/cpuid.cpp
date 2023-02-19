//@AUTOHEADER@BEGIN@
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2023 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//@AUTOHEADER@END@

#include <drift/dsl.h>

int main(int argc, char * argv[]) {
	printf("Vendor: %s\n", InstructionSet::Vendor().c_str());
	printf("Model: %s\n", InstructionSet::Brand().c_str());
	printf("SSE: %d\n", InstructionSet::SSE());
	return 0;
}
