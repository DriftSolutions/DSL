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

	vector<const HASH_PROVIDER *> p;
	dsl_get_hash_providers(p);
	printf("Num hash providers: %zu\n", p.size());

	DSL_Sockets3_OpenSSL * socks = new DSL_Sockets3_OpenSSL();
	DSL_Sockets3_GnuTLS * socks2 = new DSL_Sockets3_GnuTLS();

	dsl_get_hash_providers(p);
	printf("Num hash providers: %zu\n", p.size());

	char buf[256], buf2[256];
	string str = "Jen Towns is #1";

	map<string, string> hashtests;
	hashtests["sha512"] = "353c491fbc377105f3eec54ebb495aafe4a32db2975c132981ef5d86e453f401dcd6ef88c2c8d947d56782f99783ea78069a5c5e818ea64ade6bd97b958353bb";
	hashtests["sha256"] = "d4abc4d6b461a88d41603c634933f513045e2533b544c67f03d5b56a3d6e93c6";
	hashtests["sha1"] = "426cddde1d3699bfae66c0b3a37ef78fafb21e8f";
	hashtests["md5"] = "f8aa86a89d24c64f390fcd7a28f0eca8";

	for (auto e = p.begin(); e != p.end(); e++) {
		printf("Testing provider %s\n", (*e)->name);
		for (auto x = hashtests.begin(); x != hashtests.end(); x++) {
			memset(buf, 0, sizeof(buf));
			HASH_CTX * ptr = (*e)->hash_init(x->first.c_str());
			if (ptr != NULL) {
				(*e)->hash_update(ptr, (const uint8_t *)str.c_str(), str.length());
				size_t hlen = ptr->hashSize;
				if ((*e)->hash_finish(ptr, (uint8_t *)buf, sizeof(buf))) {
					bin2hex((const uint8_t *)buf, hlen, buf2, sizeof(buf2));
					string shouldbe = x->second;
					if (stricmp(buf2, shouldbe.c_str()) == 0) {
						printf("[%s]: success!\n", x->first.c_str());
					} else {
						printf("[%s]: error! Got %s, should be %s\n", x->first.c_str(), buf2, x->second.c_str());
					}
				} else {
					printf("[%s]: error finishing hash!\n", x->first.c_str());
				}
			} else {
				printf("[%s]: error initializing hash!\n", x->first.c_str());
			}
		}
	}

	dsl_cleanup();
	return 0;
}
