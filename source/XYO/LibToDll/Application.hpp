// Lib To Dll
// Copyright (c) 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_LIBTODLL_APPLICATION_HPP
#define XYO_LIBTODLL_APPLICATION_HPP

#ifndef XYO_LIBTODLL_DEPENDENCY_HPP
#	include <XYO/LibToDll/Dependency.hpp>
#endif

namespace XYO::LibToDll {

	class Application : public virtual IApplication {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Application);

		public:
			inline Application(){};

			void showUsage();
			void showLicense();
			void showVersion();

			int main(int cmdN, char *cmdS[]);

			static void initMemory();

			String strStrip(String str);
			int cmdSystem(char *cmd);
	};

};

#endif
