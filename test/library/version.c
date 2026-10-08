// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

// Member of library.lib, compiled with /DLIBRARY_VERSION=1 or 2 to tell a rebuilt library from an old one

int library_version(void) {
	return LIBRARY_VERSION;
}
