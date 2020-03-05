//!AUTOHEADER!BEGIN!
/***********************************************************************\
|                    Drift Standard Libraries v1.01                     |
|            Copyright 2010-2020 Drift Solutions / Indy Sams            |
| Docs and more information available at https://www.driftsolutions.dev |
|          This file released under the 3-clause BSD license,           |
|            see included DSL.LICENSE.TXT file for details.             |
\***********************************************************************/
//!AUTOHEADER!END!

#include <drift/dsl.h>

int main(int argc, char * argv[]) {
	if (!dsl_init()) {
		printf("dsl_init() failed!\n");
		return 1;
	}

	printf("Version string: %s\n", dsl_get_version_string());
	printf("OS version string: %s\n", GetOSVersion());

	DSL_Sockets3_OpenSSL * socks = new DSL_Sockets3_OpenSSL();
	DSL_Sockets3_GnuTLS * socks2 = new DSL_Sockets3_GnuTLS();

	printf("PhysFS test\n");
	PHYSFS_init(argv[0]);
	DSL_FILE * fp = RW_OpenPhysFS("test.file", "rb", true);
	if (fp) {
		fp->close(fp);
	}
	PHYSFS_deinit();

	printf("libcurl test\n");
	DSL_Download_Curl dl("http://www.shoutirc.com/getip.php");
	if (!dl.Download(stdout)) {
		printf("Download error: %s", dl.GetErrorString());
	}
	printf("\n");

	DB_SQLite db_sqlite;
	DB_MySQL db_mysql;

	delete socks2;
	delete socks;

	printf("DSL cleanup\n");
	dsl_cleanup();
	printf("Success!\n");
	return 0;
}
