// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

// Uses the DLL made by lib-to-dll through its import library, exit code 0 if every call works
// use-library [version] - also checks library_version()

#include <stdio.h>
#include <stdlib.h>

__declspec(dllimport) int add_numbers(int a, int b);
__declspec(dllimport) int scale_number(int a);
__declspec(dllimport) int library_version(void);
#ifndef USE_DEF
__declspec(dllimport) int multiply_numbers(int a, int b);
#endif

int main(int argc, char *argv[]) {
	if (add_numbers(2, 3) != 5) {
		printf("add_numbers failed\n");
		return 1;
	};
	if (scale_number(7) != 21) {
		printf("scale_number failed\n");
		return 1;
	};
#ifndef USE_DEF
	if (multiply_numbers(4, 5) != 20) {
		printf("multiply_numbers failed\n");
		return 1;
	};
#endif
	if (argc > 1) {
		if (library_version() != atoi(argv[1])) {
			printf("library_version failed, %d not %s\n", library_version(), argv[1]);
			return 1;
		};
	};
	printf("use-library ok\n");
	return 0;
}
