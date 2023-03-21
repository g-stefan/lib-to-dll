// Lib To Dll
// Copyright (c) 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_LIBTODLL_COPYRIGHT_HPP
#define XYO_LIBTODLL_COPYRIGHT_HPP

#ifndef XYO_LIBTODLL_DEPENDENCY_HPP
#	include <XYO/LibToDll/Dependency.hpp>
#endif

namespace XYO::LibToDll::Copyright {
	std::string copyright();
	std::string publisher();
	std::string company();
	std::string contact();
};

#endif
