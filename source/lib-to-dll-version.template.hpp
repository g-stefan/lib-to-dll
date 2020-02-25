//
// Lib To Dll
//
// Copyright (c) 2020 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef LIB_TO_DLL_VERSION_HPP
#define LIB_TO_DLL_VERSION_HPP

#define LIB_TO_DLL_VERSION_ABCD                $VERSION_ABCD
#define LIB_TO_DLL_VERSION_STR                 "$VERSION_VERSION"
#define LIB_TO_DLL_VERSION_STR_BUILD           "$VERSION_BUILD"
#define LIB_TO_DLL_VERSION_STR_DATETIME        "$VERSION_DATETIME"

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

