// Lib To Dll
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
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
		       "    lib-to-dll [options] in.lib [extra obj/lib ...]\n\n"
		       "options:\n"
		       "    --mode type     {WIN32|WIN64} default is the target of the\n"
		       "                    Visual C++ environment, else WIN32\n"
		       "    --use-coff-def  use symbols from obj not from def file\n"
		       "    --static-crt    use static crt [MT] default is dynamic crt [MD]\n"
		       "    --license       show license\n"
		       "    --version       show version\n"
		       "    --help          show this help\n");
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

	// File names with spaces as one argument of lib, link and xyo-coff-to-def
	String Application::quote(const String &value) {
		if (value.itContains(" ") || value.itContains("\t")) {
			String retV = "\"";
			retV << value;
			retV << "\"";
			return retV;
		};
		return value;
	};

	// Shell::execute returns 127 when the process can not be started
	void Application::showCommandError(uint32_t exitCode) {
		if (exitCode == 127) {
			printf("Error: unable to run the command, lib, link and xyo-coff-to-def must be on the PATH (Visual C++ environment)\n");
			return;
		};
		printf("Error: command failed, exit code %u\n", exitCode);
	};

	// Run without cmd.exe: no quoting rules of the shell, command line up to 32767 characters
	bool Application::execute(const String &cmd) {
		printf("%s\n", cmd.value());
		fflush(stdout);
		uint32_t exitCode = Shell::execute(cmd);
		if (exitCode != 0) {
			showCommandError(exitCode);
			return false;
		};
		return true;
	};

	// The target of the Visual C++ environment, set by vcvarsall.bat
	bool Application::isDefaultModeWin64() {
		return Shell::getEnv("VSCMD_ARG_TGT_ARCH").toLowerCaseASCII() == "x64";
	};

	// Member names of a library, lib /list
	bool Application::listMembers(const String &library, const String &listFile, TDynamicArray<String> &memberList) {
		String cmd;
		String content;
		TDynamicArray<String> lineList;
		size_t k;

		memberList.empty();

		cmd = "lib /nologo /list ";
		cmd << quote(library);
		printf("%s\n", cmd.value());
		fflush(stdout);
		uint32_t exitCode = Shell::executeWriteOutputToFile(cmd, listFile);
		if (!Shell::fileGetContents(listFile, content)) {
			content = "";
		};
		if (exitCode != 0) {
			if (!content.isEmpty()) {
				printf("%s\n", content.value());
			};
			showCommandError(exitCode);
			printf("Error: unable to list the members of %s\n", library.value());
			return false;
		};

		content.explode("\n", lineList);
		for (k = 0; k < lineList.length(); ++k) {
			String line = strStrip(lineList[k]);
			if (line.length() == 0) {
				continue;
			};
			memberList.push(line);
		};
		return true;
	};

	// Every member of an import library is named after the dll (or exe) it imports from
	bool Application::isImportLibrary(TDynamicArray<String> &memberList) {
		size_t k;
		if (memberList.length() == 0) {
			return false;
		};
		for (k = 0; k < memberList.length(); ++k) {
			String member = memberList[k].toLowerCaseASCII();
			if (!(member.endsWith(".dll") || member.endsWith(".exe"))) {
				return false;
			};
		};
		return true;
	};

	// File name of an extracted member, its folders become part of the name:
	// "_" is doubled first, so two different member names do not give the same file name
	String Application::memberFileName(const String &member) {
		String retV = member;
		retV = retV.replace(".\\", "");
		retV = retV.replace("_", "__");
		retV = retV.replace("\\", "_");
		retV = retV.replace("/", "_");
		retV = retV.replace(":", "_");
		return retV;
	};

	int Application::main(int cmdN, char *cmdS[]) {
		int i;
		size_t k;
		size_t m;
		char *opt;

		String mainLib;
		TDoubleEndedQueue<String> libList;
		TDoubleEndedQueue<String>::Node *libFile;
		TDynamicArray<String> memberList;
		TDynamicArray<String> objList;
		String line;
		bool hasDef;
		bool isWin64;
		bool useCoffDef;
		bool useStaticCrt;
		bool isInfo;

		if (cmdN < 2) {
			showUsage();
			return 0;
		};

		useStaticCrt = false;
		useCoffDef = false;
		isWin64 = isDefaultModeWin64();
		isInfo = false;
		for (i = 1; i < cmdN; ++i) {
			if (strncmp(cmdS[i], "--", 2) == 0) {
				opt = &cmdS[i][2];
				if (strcmp(opt, "help") == 0 || strcmp(opt, "usage") == 0) {
					showUsage();
					return 0;
				};
				if (strcmp(opt, "license") == 0) {
					showLicense();
					isInfo = true;
					continue;
				};
				if (strcmp(opt, "version") == 0) {
					showVersion();
					isInfo = true;
					continue;
				};
				if (strcmp(opt, "use-coff-def") == 0) {
					useCoffDef = true;
					continue;
				};
				if (strcmp(opt, "static-crt") == 0) {
					useStaticCrt = true;
					continue;
				};
				if (strcmp(opt, "mode") == 0) {
					if (i + 1 >= cmdN) {
						printf("Error: --mode needs a value, WIN32 or WIN64\n");
						return 1;
					};
					++i;
					String mode = String(cmdS[i]).toUpperCaseASCII();
					if (mode == "WIN32") {
						isWin64 = false;
						continue;
					};
					if (mode == "WIN64") {
						isWin64 = true;
						continue;
					};
					printf("Error: unknown mode %s, use WIN32 or WIN64\n", cmdS[i]);
					return 1;
				};
				printf("Error: unknown option %s\n", cmdS[i]);
				return 1;
			};
			if (mainLib.isEmpty()) {
				mainLib = cmdS[i];
				continue;
			};
			libList.pushToTail(cmdS[i]);
		};

		if (mainLib.isEmpty()) {
			if (isInfo) {
				return 0;
			};
			printf("Error: no library specified\n");
			return 1;
		};

		// in.lib or in, only .lib is removed (in.v1 is a name, not in + extension)
		if (mainLib.toLowerCaseASCII().endsWith(".lib")) {
			mainLib = mainLib.substring(0, mainLib.length() - 4);
		};

		String libraryFile = mainLib + ".lib";
		String staticLibraryFile = mainLib + ".static.lib";
		String listFile = mainLib + ".lst";
		String objectPath = mainLib + ".obj";
		String defFile = mainLib + ".def";
		String dllDefFile = mainLib + ".dll.def";
		String expFile = mainLib + ".exp";
		String linkFile = mainLib + ".rsp";
		String dllFile = mainLib + ".dll";

		// The static library is kept as in.static.lib, in.lib becomes the import library of in.dll.
		// in.lib is the import library when converted before: use in.static.lib.
		// in.lib is a static library (new or rebuilt): it replaces in.static.lib.
		if (Shell::fileExists(libraryFile)) {
			if (!listMembers(libraryFile, listFile, memberList)) {
				Shell::removeFile(listFile);
				return 1;
			};
			if (isImportLibrary(memberList)) {
				if (!Shell::fileExists(staticLibraryFile)) {
					Shell::removeFile(listFile);
					printf("Error: %s is an import library and %s not found\n", libraryFile.value(), staticLibraryFile.value());
					return 1;
				};
			} else {
				if (Shell::fileExists(staticLibraryFile)) {
					printf("remove %s\n", staticLibraryFile.value());
					if (!Shell::removeFile(staticLibraryFile)) {
						Shell::removeFile(listFile);
						printf("Error: unable to remove %s\n", staticLibraryFile.value());
						return 1;
					};
				};
				printf("move %s %s\n", libraryFile.value(), staticLibraryFile.value());
				if (!Shell::rename(libraryFile, staticLibraryFile)) {
					Shell::removeFile(listFile);
					printf("Error: unable to move %s to %s\n", libraryFile.value(), staticLibraryFile.value());
					return 1;
				};
			};
		} else if (!Shell::fileExists(staticLibraryFile)) {
			printf("Error: %s not found\n", libraryFile.value());
			return 1;
		};

		hasDef = false;
		if (!useCoffDef) {
			if (Shell::fileExists(defFile)) {
				hasDef = true;
			} else {
				useCoffDef = true;
			};
		};

		// Temporary files, removed on success and on error
		auto cleanup = [&]() {
			Shell::removeDirRecursively(objectPath);
			Shell::removeFile(listFile);
			Shell::removeFile(expFile);
			Shell::removeFile(linkFile);
			if (!hasDef) {
				Shell::removeFile(dllDefFile);
			};
		};

		if (!listMembers(staticLibraryFile, listFile, memberList)) {
			cleanup();
			return 1;
		};

		// lib /extract takes the first member with a name, the others with the same name can not be extracted
		for (k = 0; k < memberList.length(); ++k) {
			for (m = 0; m < objList.length(); ++m) {
				if (objList[m] == memberList[k]) {
					break;
				};
			};
			if (m < objList.length()) {
				printf("Warning: %s has more than one member named %s, only the first one is used\n", staticLibraryFile.value(), memberList[k].value());
				continue;
			};
			objList.push(memberList[k]);
		};

		if (objList.length() == 0) {
			cleanup();
			printf("Error: %s has no members\n", staticLibraryFile.value());
			return 1;
		};

		if (Shell::directoryExists(objectPath)) {
			if (!Shell::removeDirRecursively(objectPath)) {
				cleanup();
				printf("Error: unable to remove %s\n", objectPath.value());
				return 1;
			};
		};
		if (!Shell::mkdirRecursivelyIfNotExists(objectPath)) {
			cleanup();
			printf("Error: unable to create %s\n", objectPath.value());
			return 1;
		};

		for (k = 0; k < objList.length(); ++k) {
			line = "lib /nologo ";
			line << (isWin64 ? "/MACHINE:X64" : "/MACHINE:X86");
			line << " " << quote(String("/extract:") + objList[k]);
			line << " " << quote(String("/out:") + objectPath + "\\" + memberFileName(objList[k]));
			line << " " << quote(staticLibraryFile);
			if (!execute(line)) {
				cleanup();
				return 1;
			};
		};

		if (useCoffDef) {
			line = "xyo-coff-to-def --out ";
			line << quote(dllDefFile);
			line << (isWin64 ? " --mode WIN64" : " --mode WIN32");
			for (k = 0; k < objList.length(); ++k) {
				line << " " << quote(objectPath + "\\" + memberFileName(objList[k]));
			};
			if (!execute(line)) {
				cleanup();
				return 1;
			};
		};

		// Options and objects in a response file, no limit on the number of objects
		line = "/NOLOGO ";
		line << quote(String("/OUT:") + dllFile);
		line << (isWin64 ? " /MACHINE:X64" : " /MACHINE:X86");
		if (useStaticCrt) {
			line << " /nodefaultlib:msvcrt /defaultlib:libcmt";
		} else {
			line << " /nodefaultlib:libcmt /defaultlib:msvcrt";
		};
		line << " /dll /INCREMENTAL:NO ";
		line << quote(String("/DEF:") + (hasDef ? defFile : dllDefFile));
		line << " " << quote(String("/implib:") + libraryFile);
		line << "\n";
		for (k = 0; k < objList.length(); ++k) {
			line << quote(objectPath + "\\" + memberFileName(objList[k]));
			line << "\n";
		};
		for (libFile = libList.head; libFile; libFile = libFile->next) {
			line << quote(libFile->value);
			line << "\n";
		};
		if (!Shell::filePutContents(linkFile, line)) {
			cleanup();
			printf("Error: unable to write %s\n", linkFile.value());
			return 1;
		};
		printf("%s", line.value());
		if (!execute(String("link @") + quote(linkFile))) {
			cleanup();
			return 1;
		};

		cleanup();

		printf("Build %s ok\n", dllFile.value());
		return 0;
	};
};

#ifndef XYO_LIBTODLL_LIBRARY
XYO_APPLICATION_MAIN(XYO::LibToDll::Application);
#endif
