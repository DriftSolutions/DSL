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

class SerializationTest : public DSL_Serializable {
public:
	string test1;
	char test2[64] = { 0 };
	int test3 = 0;

protected:
	bool Serialize(DSL_BUFFER * buf, bool deserialize) {
		ser(&test1);
		ser2(test2, sizeof(test2));
		ser(&test3);
		return true;
	}
};

int main(int argc, char * argv[]) {
	if (!dsl_init()) {
		printf("dsl_init() failed!\n");
		return 1;
	}

	SerializationTest test1,test2;

	test1.test1 = "Hello";
	sstrcpy(test1.test2, "World");
	test1.test3 = 42;

	string encoded = test1.GetSerialized();
	if (encoded.length()) {
		if (test2.FromSerialized(encoded)) {
			if (test2.test1 != "Hello") {
				printf("Deserialized STL string mismatch!\n");
			} else if (strcmp(test2.test2, "World")) {
				printf("Deserialized char array string mismatch!\n");
			} else if (test2.test3 != 42) {
				printf("Deserialized integer mismatch!\n");
			} else {
				printf("Serializer test success!\n");
			}
		} else {
			printf("Error deserializing data!\n");
		}
	} else {
		printf("Error serializing data!\n");
	}

	dsl_cleanup();
	return 0;
}
