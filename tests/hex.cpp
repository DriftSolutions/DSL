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
	if (!dsl_init()) {
		printf("dsl_init() failed!\n");
		return 1;
	}

	char buf[256]={0};
	char buf2[256]={0};
	string str = "Jen Towns is #1";

	if (bin2hex((uint8 *)str.c_str(), str.length(), buf, sizeof(buf)) == NULL) {
		printf("Error converting bin2hex!\n");
		return 1;
	}
	printf("hex: %s\n", buf);

	if (!hex2bin(buf, strlen(buf), (uint8 *)buf2, sizeof(buf2))) {
		printf("Error converting hex2bin!\n");
		return 1;
	}
	printf("decoded: %s\n", buf2);

	if (strcmp(buf2, str.c_str())) {
		printf("Data mismatch!\n");
		return 1;
	}

	dsl_cleanup();
	return 0;
}
