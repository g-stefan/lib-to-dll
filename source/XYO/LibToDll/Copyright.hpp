// Lib To Dll
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_LIBTODLL_COPYRIGHT_HPP
#define XYO_LIBTODLL_COPYRIGHT_HPP

#ifndef XYO_LIBTODLL_DEPENDENCY_HPP
#	include <XYO/LibToDll/Dependency.hpp>
#endif

namespace XYO::LibToDll::Copyright {
	const char *copyright();
	const char *publisher();
	const char *company();
	const char *contact();
};

#endif
