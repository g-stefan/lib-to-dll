// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

// Member of library.lib with "_" in its name, uses helper.lib (extra library on the command line)

int multiply_numbers(int a, int b);
int helper_factor(void);

int scale_number(int a) {
	return multiply_numbers(a, helper_factor());
}
