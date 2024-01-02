// Lib To Dll
// Copyright (c) 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_LIBTODLL_VERSION_HPP
#define XYO_LIBTODLL_VERSION_HPP

#ifndef XYO_LIBTODLL_DEPENDENCY_HPP
#	include <XYO/LibToDll/Dependency.hpp>
#endif

namespace XYO::LibToDll::Version {

	const char *version();
	const char *build();
	const char *versionWithBuild();
	const char *datetime();

};

#endif
