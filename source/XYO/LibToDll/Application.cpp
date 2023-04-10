// Lib To Dll
// Copyright (c) 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <XYO/LibToDll/Application.hpp>
#include <XYO/LibToDll/Copyright.hpp>
#include <XYO/LibToDll/License.hpp>
#include <XYO/LibToDll/Version.hpp>

namespace XYO::LibToDll {

	void Application::showUsage() {
		printf("lib-to-dll - Convert lib to dll\n");
		printf("version %s build %s [%s]\n", LibToDll::Version::version(), LibToDll::Version::build(), LibToDll::Version::datetime());
		printf("%s\n\n", LibToDll::Copyright::copyright());
		printf("%s\n",
		       "usage:\n"
		       "    lib-to-dll in.lib [extra obj/lib]\n\n"
		       "options:\n"
		       "    --mode type     {WIN32|WIN64}\n"
		       "    --license       show license\n"
		       "    --use-coff-def  use symbols form obj not from def file\n"
		       "    --static-crt    use static crt [MT] default is dynamic crt [MD]\n");
	};

	void Application::showLicense() {
		printf("%s", LibToDll::License::license().c_str());
	};

	void Application::showVersion() {
		printf("version %s build %s [%s]\n", LibToDll::Version::version(), LibToDll::Version::build(), LibToDll::Version::datetime());
	};

	void Application::initMemory() {
		String::initMemory();
		TDynamicArray<String>::initMemory();
		TDoubleEndedQueue<String>::initMemory();
	};

	String Application::strStrip(String str) {
		return str.trimWithElement("\r\n\t ");
	};

	int Application::cmdSystem(char *cmd) {
		printf("%s\n", cmd);
		return system(cmd);
	};

	int Application::main(int cmdN, char *cmdS[]) {
		int i;
		FILE *in;
		int found;
		char ch;
		char *opt;
		char buf[1024];

		char *mainLib;
		TDoubleEndedQueue<String> libList;
		String line;
		TDoubleEndedQueue<String> objList;
		TDoubleEndedQueue<String>::Node *objFile;
		String objAs;
		String objX;
		int hasDef;
		String mainLibX;
		int coffMode;
		int useCoffDef;
		int useStaticCrt;

		if (cmdN < 2) {
			showUsage();
			return 0;
		};

		useStaticCrt = 0;
		useCoffDef = 0;
		coffMode = 0;
		mainLib = NULL;
		libList.empty();
		for (i = 1; i < cmdN; ++i) {
			if (strncmp(cmdS[i], "--", 2) == 0) {
				opt = &cmdS[i][2];
				if (strcmp(opt, "license") == 0) {
					showLicense();
				} else if (strcmp(opt, "use-coff-def") == 0) {
					useCoffDef = 1;
				} else if (strcmp(opt, "static-crt") == 0) {
					useStaticCrt = 1;
				} else if (strcmp(opt, "mode") == 0) {
					if (i + 1 < cmdN) {
						if (strcmp(cmdS[i + 1], "WIN32") == 0) {
							coffMode = 0;
						};
						if (strcmp(cmdS[i + 1], "WIN64") == 0) {
							coffMode = 1;
						};
						++i;
					};
					continue;
				};
			} else {
				if (mainLib) {
					libList.pushToTail(cmdS[i]);
				} else {
					mainLib = cmdS[i];
				};
			};
		};

		if (mainLib) {
			size_t last;
			mainLibX = (char *)mainLib;
			if (mainLibX.indexOfFromEnd(".", 0, last)) {
				mainLibX = mainLibX.substring(0, last);
				mainLib = (char *)mainLibX.value();
			};
		} else {
			printf("No library specified.\n");
			return 0;
		};

		sprintf(buf, "if not exist %s.static.lib move /Y %s.lib %s.static.lib", mainLib, mainLib, mainLib);
		if (cmdSystem(buf)) {
			return 1;
		}
		sprintf(buf, "lib /nologo /list %s.static.lib >%s.lst", mainLib, mainLib);
		if (cmdSystem(buf)) {
			return 1;
		}

		objList.empty();
		sprintf(buf, "%s.lst", mainLib);
		in = fopen(buf, "rt");
		if (in != NULL) {
			while (fgets(buf, 1024, in)) {
				line = strStrip((char *)&buf[0]);
				if (line.length() == 0) {
					continue;
				};
				objList.pushToTail(line);
			};
			fclose(in);
		};

		sprintf(buf, "if exist %s.obj\\NUL rmdir /S /Q %s.obj", mainLib, mainLib);
		if (cmdSystem(buf)) {
			return 1;
		}
		sprintf(buf, "if not exist %s.obj\\NUL mkdir %s.obj", mainLib, mainLib);
		if (cmdSystem(buf)) {
			return 1;
		}
		for (objFile = objList.head; objFile; objFile = objFile->next) {
			objAs = objFile->value;
			objX = objFile->value;
			objAs = objAs.replace(".\\", "");
			objAs = objAs.replace("_", "__");
			objAs = objAs.replace("\\", "_");
			objAs = objAs.replace("/", "_");
			objAs = objAs.replace(":", "_");
			if (coffMode) {
				sprintf(buf, "lib /nologo /MACHINE:X64 /extract:%s /out:%s.obj\\%s %s.static.lib", objX.value(), mainLib, objAs.value(), mainLib);
			} else {
				sprintf(buf, "lib /nologo /MACHINE:X86 /extract:%s /out:%s.obj\\%s %s.static.lib", objX.value(), mainLib, objAs.value(), mainLib);
			};
			if (cmdSystem(buf)) {
				return 1;
			}
		};

		hasDef = 0;
		if (useCoffDef) {
		} else {
			sprintf(buf, "%s.def", mainLib);
			in = fopen(buf, "rt");
			if (in) {
				fclose(in);
				hasDef = 1;
			} else {
				useCoffDef = 1;
			};
		};

		if (useCoffDef) {

			line = "xyo-coff-to-def --out ";
			line << (char *)mainLib;
			if (coffMode) {
				line << ".dll.def --mode WIN64 ";
			} else {
				line << ".dll.def --mode WIN32 ";
			};
			for (objFile = objList.head; objFile; objFile = objFile->next) {
				objAs = objFile->value;
				objAs = objAs.replace(".\\", "");
				objAs = objAs.replace("_", "__");
				objAs = objAs.replace("\\", "_");
				objAs = objAs.replace("/", "_");
				objAs = objAs.replace(":", "_");
				line += mainLib;
				line += ".obj\\";
				line += objAs;
				line += " ";
			};
			if (cmdSystem((char *)line.value())) {
				return 1;
			}
		};

		line = "link /NOLOGO /OUT:";
		line << mainLib;
		line << ".dll ";
		if (coffMode) {
			line << "/MACHINE:X64 ";
		} else {
			line << "/MACHINE:X86 ";
		};
		if (useStaticCrt) {
			line << "/nodefaultlib:msvcrt /defaultlib:libcmt ";
		} else {
			line << "/nodefaultlib:libcmt /defaultlib:msvcrt ";
		};

		line << "/dll /INCREMENTAL:NO /DEF:";
		line << mainLib;
		if (hasDef) {
			line << ".def /implib:";
		} else {
			line << ".dll.def /implib:";
		};
		line << mainLib;
		line << ".lib ";

		for (objFile = objList.head; objFile; objFile = objFile->next) {
			objAs = objFile->value;
			objAs = objAs.replace(".\\", "");
			objAs = objAs.replace("_", "__");
			objAs = objAs.replace("\\", "_");
			objAs = objAs.replace("/", "_");
			objAs = objAs.replace(":", "_");
			line << mainLib;
			line << ".obj\\";
			line << objAs;
			line << " ";
		};

		for (objFile = libList.head; objFile; objFile = objFile->next) {
			objAs = objFile->value;
			line << objAs;
			line << " ";
		};

		if (cmdSystem((char *)line.value())) {
			return 1;
		}

		sprintf(buf, "if exist %s.obj\\NUL rmdir /S /Q %s.obj", mainLib, mainLib);
		if (cmdSystem(buf)) {
			return 1;
		}
		sprintf(buf, "if exist %s.lst del /F /Q %s.lst", mainLib, mainLib);
		if (cmdSystem(buf)) {
			return 1;
		}
		sprintf(buf, "if exist %s.exp del /F /Q %s.exp", mainLib, mainLib);
		if (cmdSystem(buf)) {
			return 1;
		}
		if (hasDef) {
		} else {
			sprintf(buf, "if exist %s.dll.def del /F /Q %s.dll.def", mainLib, mainLib);
			if (cmdSystem(buf)) {
				return 1;
			}
		};

		printf("Build %s.dll ok\n", mainLib);
		return 0;
	};
};

#ifndef XYO_LIBTODLL_LIBRARY
XYO_APPLICATION_MAIN(XYO::LibToDll::Application);
#endif
