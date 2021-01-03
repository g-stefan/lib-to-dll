//
// Lib To Dll
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef LIB_TO_DLL_VERSION_HPP
#define LIB_TO_DLL_VERSION_HPP

#define LIB_TO_DLL_VERSION_ABCD                1,2,0,5
#define LIB_TO_DLL_VERSION_STR                 "1.2.0"
#define LIB_TO_DLL_VERSION_STR_BUILD           "5"
#define LIB_TO_DLL_VERSION_STR_DATETIME        "2021-01-03 02:30:56"

#ifndef XYO_RC

namespace LibToDll {
	namespace Version {
		const char *version();
		const char *build();
		const char *versionWithBuild();
		const char *datetime();
	};
};

#endif
#endif

