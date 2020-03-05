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

	vector<const HMAC_PROVIDER *> p;
	dsl_get_hmac_providers(p);
	printf("Num hmac providers: %zu\n", p.size());

	DSL_Sockets3_OpenSSL * socks = new DSL_Sockets3_OpenSSL();
	DSL_Sockets3_GnuTLS * socks2 = new DSL_Sockets3_GnuTLS();

	dsl_get_hmac_providers(p);
	printf("Num hmac providers: %zu\n", p.size());

	char buf[256], buf2[256];
	string str = "Jen Towns is #1";
	string key = "xyz123";

	map<string, string> hmactests;
	hmactests["sha512"] = "4bcc6ef669aeea548ad03f441b596456592422e52e09c4411a47f6d93d8d1a0f9860d91a71785075c9b9edac8e3219ef714964a5532736ab5b92d4ba2889d8f5";
	hmactests["sha256"] = "da21cb4b27b05bbf7eedb93c59b8da5dd5d8300607eeba4922f15b91f43a7852";
	hmactests["sha1"] = "5ca9d96eab2c7011eb2787226f85b30cc26d3af8";
	hmactests["md5"] = "abad86a127b4262a1ad8e5ae6b9fad01";

	for (auto e = p.begin(); e != p.end(); e++) {
		printf("Testing provider %s\n", (*e)->name);
		for (auto x = hmactests.begin(); x != hmactests.end(); x++) {
			memset(buf, 0, sizeof(buf));
			HASH_HMAC_CTX * ptr = (*e)->hmac_init(x->first.c_str(), (const uint8_t *)key.c_str(), key.length());
			if (ptr != NULL) {
				(*e)->hmac_update(ptr, (const uint8_t *)str.c_str(), str.length());
				size_t hlen = ptr->hashSize;
				if ((*e)->hmac_finish(ptr, (uint8_t *)buf, sizeof(buf))) {
					bin2hex((const uint8_t *)buf, hlen, buf2, sizeof(buf2));
					string shouldbe = x->second;
					if (stricmp(buf2, shouldbe.c_str()) == 0) {
						printf("[%s]: success!\n", x->first.c_str());
					} else {
						printf("[%s]: error! Got %s, should be %s\n", x->first.c_str(), buf2, x->second.c_str());
					}
				} else {
					printf("[%s]: error finishing hmac!\n", x->first.c_str());
				}
			} else {
				printf("[%s]: error initializing hmac!\n", x->first.c_str());
			}
		}
	}

	dsl_cleanup();
	return 0;
}
