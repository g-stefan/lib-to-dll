@echo off
rem Public domain
rem http://unlicense.org/
rem Created by Grigore Stefan <g_stefan@yahoo.com>

echo -^> sign lib-to-dll

pushd output
for /r %%i in (*.exe) do call grigore-stefan.sign "LIB to DLL" "%%i"
popd
