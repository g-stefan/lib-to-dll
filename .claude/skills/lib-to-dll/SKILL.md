---
name: lib-to-dll
description: >-
  How to use lib-to-dll, the XYO Windows command line tool (namespace
  XYO::LibToDll, on top of xyo-system) that converts a Visual C++ static
  library (in.lib) into a DLL plus import library without changing the
  library's code: lib /list + lib /extract of every member, xyo-coff-to-def
  for the exports (or the user's in.def), link /dll with /implib:in.lib;
  the result in.dll, in.lib (now the import library) and in.static.lib (the
  original); options --mode WIN32|WIN64 (default from VSCMD_ARG_TGT_ARCH,
  else WIN32), --use-coff-def, --static-crt, --license, --version, --help;
  extra .lib / .obj arguments linked into the DLL; converting again after a
  rebuild; exit codes and messages; LNK1112 / LNK2019 / CRT mismatch / /GL /
  dllimport for variables / duplicate member names. Use when converting a
  static library to a DLL, running lib-to-dll in a build or fabricare script
  (vendor-* projects), debugging its output, or working inside the
  lib-to-dll repository.
---

# lib-to-dll

Converts a static library built with Visual C++ into a DLL. It has no
conversion code of its own: it drives `lib.exe`, `link.exe` and
`xyo-coff-to-def`. Windows + Visual C++ environment only.

Full documentation: `docs/` in the lib-to-dll repository
(`X:\Storage\XYO\Gitea\CPP\lib-to-dll\docs` on this machine): README,
getting-started, command-line, **how-it-works**, troubleshooting. The
whole tool is `source/XYO/LibToDll/Application.cpp`.

## Usage

```
lib-to-dll [--mode WIN32|WIN64] [--use-coff-def] [--static-crt] in.lib [extra.lib | extra.obj ...]
```

| Before | After a successful run (exit `0`, last line `Build in.dll ok`) |
|--------|------------------------------------------------------------------|
| `in.lib` static | `in.dll`; `in.lib` = **import library** of `in.dll`; `in.static.lib` = the original static library |

Programs keep linking `in.lib` and now need `in.dll` next to them.

## Hard rules

1. **Run inside a Visual C++ environment** (`vcvarsall.bat x64`, Developer
   Command Prompt, or a fabricare build) with `xyo-coff-to-def` on the
   `PATH`. Otherwise: `Error: unable to run the command, ... must be on the
   PATH`, exit `1`.
2. **Mode = machine of the objects.** `--mode WIN64` for x64, `--mode WIN32`
   for x86, any letter case; no ARM64. Default: `VSCMD_ARG_TGT_ARCH=x64`
   → `WIN64`, anything else → `WIN32` (older versions: always `WIN32`).
   Wrong mode → `link` `LNK1112 ... conflicts with target machine type`,
   exit `1`. **In scripts always pass `--mode`**, e.g.
   `(Platform.osType == "win64") ? "WIN64" : "WIN32"` in fabricare.
3. **Options take two dashes**; unknown `--x` → `Error: unknown option`,
   exit `1`. `-mode` (one dash) is taken as a file name.
4. **First non option argument = library**, later ones = extra `.lib` /
   `.obj` given to `link` (dependencies of the library: `zlib.lib`,
   `ws2_32.lib`, ...). Missing dependencies → `LNK2019` unresolved external.
5. **Only a final `.lib` is stripped** from the name (`zlib.v1` →
   `zlib.v1.lib`). Files next to it: `in.static.lib`, `in.dll`, `in.def`
   (user's), temporary `in.obj\`, `in.lst`, `in.rsp`, `in.exp`,
   `in.dll.def` (removed after success **and** after an error).
6. **Exports.** `in.def` next to the library → used as is (never changed).
   No `in.def`, or `--use-coff-def` → `xyo-coff-to-def` exports every
   external symbol of the objects (functions, variables, C++ decorated
   names, inline / template instances too).
7. **C runtime must match the objects.** Default `/defaultlib:msvcrt`
   (objects `/MD`); `--static-crt` → `/defaultlib:libcmt` (objects `/MT`),
   no VCRUNTIME dependency. Debug runtimes (`/MDd`, `/MTd`) are not
   selected: build the library in release.
8. **Variables exported from the DLL need `__declspec(dllimport)`** in the
   program's declaration; functions do not.
9. **Converting again is safe.** `in.lib` is checked with `lib /list`: an
   import library (every member named `*.dll` / `*.exe`) → `in.static.lib`
   is used; a static library (fresh build) → it **replaces**
   `in.static.lib`. The static library is moved, never deleted, so after a
   failure the next run still has it.
10. **Objects compiled with `/GL`** can not be read by `xyo-coff-to-def`:
    build without `/GL` or write `in.def`.
11. **Duplicate member names** (same stored path twice) → `Warning: ... only
    the first one is used`; the other object is missing from the DLL.

## Exit codes and messages

`0`: DLL made, or no arguments / `--help` / `--license` / `--version`
without a library (`--license`, `--version` print and go on). `1`: any
error, one `Error: ...` line: `unknown option`, `--mode needs a value`,
`unknown mode`, `no library specified`, `in.lib not found`, `in.lib is an
import library and in.static.lib not found`, `unable to list the members`,
`has no members`, `command failed, exit code N` (tool messages above it),
`unable to remove / move / create / write`.

## Examples

```
lib-to-dll --mode WIN64 zlib.lib
lib-to-dll --mode WIN32 libpng.lib zlib.lib
lib-to-dll --mode WIN64 --static-crt sqlite3.lib          (with sqlite3.def next to it: only those exports)
lib-to-dll --mode WIN64 "C:\Build Output\mylib.lib"
```

fabricare script:

```js
var mode = (Platform.osType == "win64") ? "WIN64" : "WIN32";
exitIf(Shell.system("lib-to-dll --mode " + mode + " output\\lib\\mylib.lib"));
```

## Working on the repository

- Build / test with fabricare (see the `fabricare` skill):
  `fabricare make`, then `fabricare test` (`fabricare/test.js`: compiles
  the C sources in `test/` into a static library, converts it in many
  situations, links and runs `test/use-library.c` against the DLL, checks
  exports and dependents with `dumpbin`), `fabricare clean`.
- Commands run through `Shell::execute` (no `cmd.exe`, up to 32767
  characters), file names with spaces are quoted by `quote()`; `link` gets
  its arguments in the response file `in.rsp`.
- Source files use CRLF; docs and this skill use LF. License: code MIT,
  `test/`, `fabricare/`, `.claude/` Unlicense. Every new top level file or
  folder needs a `Files:` entry in `.reuse/dep5` (check with
  `python -m reuse lint`).
