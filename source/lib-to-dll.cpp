//
// Lib To Dll
//
// Copyright (c) 2020 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include <stdio.h>
#include <string.h>

#include "xyo.hpp"
#include "lib-to-dll-version.hpp"
#include "lib-to-dll-license.hpp"
#include "lib-to-dll-copyright.hpp"

namespace Main {

	using namespace XYO;

	class Application :
		public virtual IMain {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(Application);
		protected:

			void showUsage();
			void showLicense();

			String strStrip(String str);
			int cmdSystem(char *cmd);
		public:
			inline Application() {};

			int main(int cmdN, char *cmdS[]);

			inline void initMemory();
	};

	String Application::strStrip(String str) {
		return String::trimWithElement(str, "\r\n\t ");
	};

	void Application::showUsage() {
		printf("lib-to-dll - Convert lib to dll\n");
		printf("version %s build %s [%s]\n", LibToDll::Version::version(), LibToDll::Version::build(), LibToDll::Version::datetime());
		printf("%s\n\n", LibToDll::Copyright::fullCopyright());
		printf("%s\n",
			"usage:\n"
			"    lib-to-dll in.lib [extra obj/lib]\n\n"
			"options:\n"
			"    --mode type     {WIN32|WIN64}\n"
			"    --license       show license\n"
			"    --use-coff-def  use symbols form obj not from def file\n"
			"    --static-crt    use static crt [MT] default is dynamic crt [MD]\n"
		);
	};

	void Application::showLicense() {
		printf("%s", LibToDll::License::content());
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

		if(cmdN < 2) {
			showUsage();
			return 0;
		};

		useStaticCrt = 0;
		useCoffDef = 0;
		coffMode = 0;
		mainLib = NULL;
		libList.empty();
		for(i = 1; i < cmdN; ++i) {
			if(strncmp(cmdS[i], "--", 2) == 0) {
				opt = &cmdS[i][2];
				if(strcmp(opt, "license") == 0) {
					showLicense();
				} else if(strcmp(opt, "use-coff-def") == 0) {
					useCoffDef = 1;
				} else if(strcmp(opt, "static-crt") == 0) {
					useStaticCrt = 1;
				} else if(strcmp(opt, "mode") == 0) {
					if(i + 1 < cmdN) {
						if(strcmp(cmdS[i + 1], "WIN32") == 0) {
							coffMode = 0;
						};
						if(strcmp(cmdS[i + 1], "WIN64") == 0) {
							coffMode = 1;
						};
						++i;
					};
					continue;
				};
			} else {
				if(mainLib) {
					libList.pushToTail(cmdS[i]);
				} else {
					mainLib = cmdS[i];
				};
			};
		};

		if(mainLib) {
			size_t last;
			mainLibX = (char *)mainLib;
			if(String::indexOfFromEnd(mainLibX, ".", 0, last)) {
				mainLibX = String::substring(mainLibX, 0, last);
				mainLib = (char *)mainLibX.value();
			};
		} else {
			printf("No library specified.\n");
			return 0;
		};

		sprintf(buf, "if not exist %s.static.lib move /Y %s.lib %s.static.lib", mainLib, mainLib, mainLib, mainLib);
		if(cmdSystem(buf)) {
			return 1;
		}
		sprintf(buf, "lib /nologo /list %s.static.lib >%s.lst", mainLib, mainLib);
		if(cmdSystem(buf)) {
			return 1;
		}

		objList.empty();
		sprintf(buf, "%s.lst", mainLib);
		in = fopen(buf, "rt");
		if(in != NULL) {
			while(fgets(buf, 1024, in)) {
				line = strStrip((char *)&buf[0]);
				if(line.length() == 0) {
					continue;
				};
				objList.pushToTail(line);
			};
			fclose(in);
		};

		sprintf(buf, "if exist %s.obj\\NUL rmdir /S /Q %s.obj", mainLib, mainLib);
		if(cmdSystem(buf)) {
			return 1;
		}
		sprintf(buf, "if not exist %s.obj\\NUL mkdir %s.obj", mainLib, mainLib);
		if(cmdSystem(buf)) {
			return 1;
		}
		for(objFile = objList.head; objFile; objFile = objFile->next) {
			objAs = objFile->value;
			objX = objFile->value;
			objAs = String::replace(objAs, ".\\", "");
			objAs = String::replace(objAs, "_", "__");
			objAs = String::replace(objAs, "\\", "_");
			objAs = String::replace(objAs, "/", "_");
			objAs = String::replace(objAs, ":", "_");
			if(coffMode) {
				sprintf(buf, "lib /nologo /MACHINE:X64 /extract:%s /out:%s.obj\\%s %s.static.lib", objX.value(), mainLib, objAs.value(), mainLib);
			} else {
				sprintf(buf, "lib /nologo /MACHINE:X86 /extract:%s /out:%s.obj\\%s %s.static.lib", objX.value(), mainLib, objAs.value(), mainLib);
			};
			if(cmdSystem(buf)) {
				return 1;
			}
		};


		hasDef = 0;
		if(useCoffDef) {
		} else {
			sprintf(buf, "%s.def", mainLib);
			in = fopen(buf, "rt");
			if(in) {
				fclose(in);
				hasDef = 1;
			} else {
				useCoffDef = 1;
			};
		};

		if(useCoffDef) {

			line = "xyo-coff-to-def --out ";
			line << (char *)mainLib;
			if(coffMode) {
				line << ".dll.def --mode WIN64 ";
			} else {
				line << ".dll.def --mode WIN32 ";
			};
			for(objFile = objList.head; objFile; objFile = objFile->next) {
				objAs = objFile->value;
				objAs = String::replace(objAs, ".\\", "");
				objAs = String::replace(objAs, "_", "__");
				objAs = String::replace(objAs, "\\", "_");
				objAs = String::replace(objAs, "/", "_");
				objAs = String::replace(objAs, ":", "_");
				line += mainLib;
				line += ".obj\\";
				line += objAs;
				line += " ";
			};
			if(cmdSystem((char *)line.value())) {
				return 1;
			}
		};

		line = "link /NOLOGO /OUT:";
		line << mainLib;
		line << ".dll ";
		if(coffMode) {
			line << "/MACHINE:X64 ";
		} else {
			line << "/MACHINE:X86 ";
		};
		if(useStaticCrt) {
			line << "/nodefaultlib:msvcrt /defaultlib:libcmt ";
		} else {
			line << "/nodefaultlib:libcmt /defaultlib:msvcrt ";
		};


		line << "/dll /INCREMENTAL:NO /DEF:";
		line << mainLib;
		if(hasDef) {
			line << ".def /implib:";
		} else {
			line << ".dll.def /implib:";
		};
		line << mainLib;
		line << ".lib ";

		for(objFile = objList.head; objFile; objFile = objFile->next) {
			objAs = objFile->value;
			objAs = String::replace(objAs, ".\\", "");
			objAs = String::replace(objAs, "_", "__");
			objAs = String::replace(objAs, "\\", "_");
			objAs = String::replace(objAs, "/", "_");
			objAs = String::replace(objAs, ":", "_");
			line << mainLib;
			line << ".obj\\";
			line << objAs;
			line << " ";
		};

		for(objFile = libList.head; objFile; objFile = objFile->next) {
			objAs = objFile->value;
			line << objAs;
			line << " ";
		};


		if(cmdSystem((char *)line.value())) {
			return 1;
		}

		sprintf(buf, "if exist %s.obj\\NUL rmdir /S /Q %s.obj", mainLib, mainLib);
		if(cmdSystem(buf)) {
			return 1;
		}
		sprintf(buf, "if exist %s.lst del /F /Q %s.lst", mainLib, mainLib);
		if(cmdSystem(buf)) {
			return 1;
		}
		sprintf(buf, "if exist %s.exp del /F /Q %s.exp", mainLib, mainLib);
		if(cmdSystem(buf)) {
			return 1;
		}
		if(hasDef) {
		} else {
			sprintf(buf, "if exist %s.dll.def del /F /Q %s.dll.def", mainLib, mainLib);
			if(cmdSystem(buf)) {
				return 1;
			}
		};

		printf("Build %s.dll ok\n", mainLib);
		return 0;
	};

	void Application::initMemory() {
		String::initMemory();
		TDoubleEndedQueue<String>::initMemory();
	};

};

XYO_APPLICATION_MAIN_STD(Main::Application);
