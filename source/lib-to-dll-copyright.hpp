//
// Lib To Dll
//
// Copyright (c) 2020 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef LIB_TO_DLL_COPYRIGHT_HPP
#define LIB_TO_DLL_COPYRIGHT_HPP

#define LIB_TO_DLL_COPYRIGHT            "Copyright (c) Grigore Stefan"
#define LIB_TO_DLL_PUBLISHER            "Grigore Stefan"
#define LIB_TO_DLL_COMPANY              LIB_TO_DLL_PUBLISHER
#define LIB_TO_DLL_CONTACT              "g_stefan@yahoo.com"
#define LIB_TO_DLL_FULL_COPYRIGHT       LIB_TO_DLL_COPYRIGHT " <" LIB_TO_DLL_CONTACT ">"

#ifndef XYO_RC

namespace LibToDll {
	namespace Copyright {
		const char *copyright();
		const char *publisher();
		const char *company();
		const char *contact();
		const char *fullCopyright();
	};
};

#endif
#endif
