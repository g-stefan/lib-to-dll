// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

// lib-to-dll runs lib.exe, link.exe and xyo-coff-to-def, the test needs the Visual C++ tools
// fabricare test runs inside the Visual C++ environment of the platform

messageAction("test");

if (Script.isNil(Platform.name) || (Platform.name.indexOf("msvc") < 0)) {
	messageAction("skip, lib-to-dll needs the Visual C++ tools");
	Script.exit(0);
};

var mode = "WIN32";
if (Platform.osType == "win64") {
	mode = "WIN64";
};

var libToDll = "output\\bin\\lib-to-dll";
var pathTest = "temp\\test";
var pathObj = pathTest + "\\obj";
var outputFile = pathTest + "\\output.txt";

Shell.removeDirRecursively(pathTest);
Shell.mkdirRecursivelyIfNotExists(pathObj + "\\sub");

// run a command through the shell, output to outputFile, return the exit code
// the command must not start with a quote (cmd /c removes it)
var run = function(cmd) {
	Shell.removeFile(outputFile);
	return Shell.system(cmd + " > " + outputFile + " 2>&1");
};

var runLibToDll = function(args) {
	return run(libToDll + " " + args);
};

var outputHas = function(text) {
	var output = Shell.fileGetContents(outputFile);
	if (Script.isNil(output)) {
		return false;
	};
	return output.indexOf(text) >= 0;
};

var showOutput = function() {
	var output = Shell.fileGetContents(outputFile);
	if (!Script.isNil(output)) {
		Console.writeLn(output);
	};
};

var check = function(isFailed, message) {
	if (isFailed) {
		showOutput();
	};
	exitIfTest(isFailed, message);
};

var compile = function(source, object, options) {
	exitIf(run("cl /nologo /c /MD " + options + " /Fo" + object + " " + source), "compile " + source);
};

// --- fixtures

compile("test\\library\\adder.c", pathObj + "\\adder.obj", "");
compile("test\\library\\sub\\multiplier.c", pathObj + "\\sub\\multiplier.obj", "");
compile("test\\library\\scale_number.c", pathObj + "\\scale_number.obj", "");
compile("test\\library\\version.c", pathObj + "\\version.1.obj", "/DLIBRARY_VERSION=1");
compile("test\\library\\version.c", pathObj + "\\version.2.obj", "/DLIBRARY_VERSION=2");
compile("test\\helper\\helper.c", pathObj + "\\helper.obj", "");
compile("test\\use-library.c", pathObj + "\\use-library.obj", "");
compile("test\\use-library.c", pathObj + "\\use-library.def.obj", "/DUSE_DEF");

exitIf(run("lib /nologo /OUT:" + pathTest + "\\helper.lib " + pathObj + "\\helper.obj"), "helper.lib");

// static library.lib in path, members keep their folders (temp\test\obj\sub\multiplier.obj)
var makeLibrary = function(path, version) {
	Shell.mkdirRecursivelyIfNotExists(path);
	exitIf(run("lib /nologo \"/OUT:" + path + "\\library.lib\" " +
	           pathObj + "\\adder.obj " +
	           pathObj + "\\sub\\multiplier.obj " +
	           pathObj + "\\scale_number.obj " +
	           pathObj + "\\version." + version + ".obj"),
	       "library.lib");
};

// link use-library.exe next to library.dll and run it
var useLibrary = function(path, useObj, version) {
	if (run("link /nologo \"/OUT:" + path + "\\use-library.exe\" " + pathObj + "\\" + useObj + " \"" + path + "\\library.lib\"")) {
		return 1;
	};
	return run("call \"" + path + "\\use-library.exe\" " + version);
};

// the outputs of a conversion, temporary files removed
var checkConversion = function(path, message) {
	check(!Shell.fileExists(path + "\\library.dll"), message + " library.dll");
	check(!Shell.fileExists(path + "\\library.lib"), message + " library.lib");
	check(!Shell.fileExists(path + "\\library.static.lib"), message + " library.static.lib");
	check(Shell.fileExists(path + "\\library.lst") || Shell.fileExists(path + "\\library.exp") || Shell.fileExists(path + "\\library.dll.def") || Shell.fileExists(path + "\\library.rsp") || Shell.directoryExists(path + "\\library.obj"), message + " temporary files removed");
};

// --- informational options, errors

check((runLibToDll("") != 0) || !outputHas("usage:"), "no arguments");
check((runLibToDll("--help") != 0) || !outputHas("usage:"), "help");
check((runLibToDll("--usage") != 0) || !outputHas("usage:"), "usage");
check((runLibToDll("--version") != 0) || !outputHas("version "), "version");
check((runLibToDll("--license") != 0) || !outputHas("MIT"), "license");

check((runLibToDll("--unknown " + pathTest + "\\none.lib") != 1) || !outputHas("Error: unknown option --unknown"), "unknown option");
check((runLibToDll("--mode WIN16 " + pathTest + "\\none.lib") != 1) || !outputHas("Error: "), "invalid mode");
check((runLibToDll(pathTest + "\\none.lib --mode") != 1) || !outputHas("Error: "), "mode without value");
check((runLibToDll("--use-coff-def") != 1) || !outputHas("Error: no library"), "no library");
check((runLibToDll(pathTest + "\\none.lib") != 1) || !outputHas("Error: "), "missing library");
check(Shell.fileExists(pathTest + "\\none.lst"), "missing library, no files left");

// --- conversion, symbols from the objects, extra library

var path = pathTest + "\\basic";
makeLibrary(path, 1);
check(runLibToDll("--mode " + mode + " " + path + "\\library.lib " + pathTest + "\\helper.lib") != 0, "convert");
checkConversion(path, "convert");
check(useLibrary(path, "use-library.obj", 1) != 0, "use dll");

// again, library.lib is now the import library: library.static.lib is used
check(runLibToDll("--mode " + mode + " " + path + "\\library.lib " + pathTest + "\\helper.lib") != 0, "convert again");
checkConversion(path, "convert again");
check(useLibrary(path, "use-library.obj", 1) != 0, "convert again, use dll");

// library.lib rebuilt as a static library: it replaces the old library.static.lib
makeLibrary(path, 2);
check(runLibToDll("--mode " + mode + " " + path + "\\library.lib " + pathTest + "\\helper.lib") != 0, "rebuilt library");
checkConversion(path, "rebuilt library");
check(useLibrary(path, "use-library.obj", 2) != 0, "rebuilt library, new code");

// --- link error (helper.lib missing): exit code 1, temporary files removed, the static library kept

path = pathTest + "\\link-error";
makeLibrary(path, 1);
check((runLibToDll("--mode " + mode + " " + path + "\\library.lib") != 1) || !outputHas("Error: "), "link error");
check(Shell.fileExists(path + "\\library.lst") || Shell.fileExists(path + "\\library.exp") || Shell.fileExists(path + "\\library.dll.def") || Shell.fileExists(path + "\\library.rsp") || Shell.directoryExists(path + "\\library.obj"), "link error, temporary files removed");
check(!Shell.fileExists(path + "\\library.static.lib"), "link error, static library kept");
check(runLibToDll("--mode " + mode + " " + path + "\\library.lib " + pathTest + "\\helper.lib") != 0, "link error, convert again");
checkConversion(path, "link error, convert again");
check(useLibrary(path, "use-library.obj", 1) != 0, "link error, convert again, use dll");

// --- mode not matching the objects: exit code 1, temporary files removed

var wrongMode = (mode == "WIN64") ? "WIN32" : "WIN64";
path = pathTest + "\\wrong-mode";
makeLibrary(path, 1);
check((runLibToDll("--mode " + wrongMode + " " + path + "\\library.lib " + pathTest + "\\helper.lib") != 1) || !outputHas("Error: "), "wrong mode");
check(Shell.fileExists(path + "\\library.lst") || Shell.fileExists(path + "\\library.rsp") || Shell.directoryExists(path + "\\library.obj"), "wrong mode, temporary files removed");
check(!Shell.fileExists(path + "\\library.static.lib"), "wrong mode, static library kept");

// --- the name without .lib

path = pathTest + "\\no-extension";
makeLibrary(path, 1);
check(runLibToDll("--mode " + mode + " " + path + "\\library " + pathTest + "\\helper.lib") != 0, "no extension");
checkConversion(path, "no extension");
check(useLibrary(path, "use-library.obj", 1) != 0, "no extension, use dll");

// --- default mode, the target of the Visual C++ environment

path = pathTest + "\\default-mode";
makeLibrary(path, 1);
check(runLibToDll(path + "\\library.lib " + pathTest + "\\helper.lib") != 0, "default mode");
checkConversion(path, "default mode");
check(useLibrary(path, "use-library.obj", 1) != 0, "default mode, use dll");

// --- folder with spaces and a dot

path = pathTest + "\\folder.v1 with space";
makeLibrary(path, 1);
check(runLibToDll("--mode " + mode + " \"" + path + "\\library.lib\" " + pathTest + "\\helper.lib") != 0, "path with spaces");
checkConversion(path, "path with spaces");
check(useLibrary(path, "use-library.obj", 1) != 0, "path with spaces, use dll");

// --- library.def next to the library, only its symbols are exported

path = pathTest + "\\def";
makeLibrary(path, 1);
Shell.copyFile("test\\library\\library.def", path + "\\library.def");
check(runLibToDll("--mode " + mode + " " + path + "\\library.lib " + pathTest + "\\helper.lib") != 0, "def file");
checkConversion(path, "def file");
check(!Shell.fileExists(path + "\\library.def"), "def file kept");
check(useLibrary(path, "use-library.def.obj", 1) != 0, "def file, use dll");
check((run("dumpbin /nologo /exports " + path + "\\library.dll") != 0) || !outputHas("add_numbers") || outputHas("multiply_numbers"), "def file, exports");

// --use-coff-def: library.def is ignored, every symbol is exported
check(runLibToDll("--use-coff-def --mode " + mode + " " + path + "\\library.lib " + pathTest + "\\helper.lib") != 0, "use-coff-def");
checkConversion(path, "use-coff-def");
check(useLibrary(path, "use-library.obj", 1) != 0, "use-coff-def, use dll");
check((run("dumpbin /nologo /exports " + path + "\\library.dll") != 0) || !outputHas("multiply_numbers"), "use-coff-def, exports");

// --- static crt, no dependency on the Visual C++ runtime dll

path = pathTest + "\\static-crt";
makeLibrary(path, 1);
check(runLibToDll("--static-crt --mode " + mode + " " + path + "\\library.lib " + pathTest + "\\helper.lib") != 0, "static crt");
checkConversion(path, "static crt");
check(useLibrary(path, "use-library.obj", 1) != 0, "static crt, use dll");
check((run("dumpbin /nologo /dependents " + path + "\\library.dll") != 0) || outputHas("VCRUNTIME"), "static crt, dependents");

// --- dynamic crt (default) uses the Visual C++ runtime dll

check((run("dumpbin /nologo /dependents " + pathTest + "\\basic\\library.dll") != 0) || !outputHas("VCRUNTIME"), "dynamic crt, dependents");

Shell.removeDirRecursively(pathTest);
