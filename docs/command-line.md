# Command line

```
lib-to-dll [options] in.lib [extra.lib | extra.obj ...]
lib-to-dll --license
lib-to-dll --version
lib-to-dll --help
```

## Arguments

| Argument | Meaning |
|----------|---------|
| `in.lib` | the static library to convert, the first argument that is not an option; `.lib` can be left out (`in`) |
| `extra.lib`, `extra.obj` | every later argument that is not an option: given to `link.exe` as well, for the functions the library uses from elsewhere |

Only a final `.lib` (any letter case) is removed from the name: `zlib.v1`
is the library `zlib.v1.lib`, not `zlib` with the extension `.v1`. Every
other file is named from what is left, `in`:

| File | Role |
|------|------|
| `in.lib` | input static library; at the end the import library of `in.dll` |
| `in.static.lib` | the static library, kept |
| `in.dll` | output DLL |
| `in.def` | your DEF file, used if it exists (and not `--use-coff-def`) |
| `in.obj\`, `in.lst`, `in.rsp`, `in.exp`, `in.dll.def` | temporary, removed at the end, also after an error |

Paths may contain spaces (quote them for the shell). Relative paths are
relative to the current directory.

## Options

| Option | Effect |
|--------|--------|
| `--mode type` | `WIN32` (x86 objects, `/MACHINE:X86`) or `WIN64` (x64 objects, `/MACHINE:X64`), any letter case; see [the default mode](#the-default-mode) |
| `--use-coff-def` | export every symbol of the objects even if `in.def` exists |
| `--static-crt` | link the static C runtime (`/defaultlib:libcmt`, objects built with `/MT`); default is the DLL runtime (`/defaultlib:msvcrt`, objects built with `/MD`) |
| `--license` | print the license (MIT) |
| `--version` | print `version X build Y [date time]` |
| `--help`, `--usage` | print the usage and exit `0` |

Options can come before, between or after the file names. Two dashes:
`-mode` (one dash) is not an option, it is taken as a file name. An unknown
option is an error.

`--license` and `--version` print and go on: with no library they then exit
`0`, with a library they also convert it.

Running with **no arguments** prints the usage and exits `0`.

## The default mode

Without `--mode`, the mode is the target of the Visual C++ environment:
`vcvarsall.bat` sets `VSCMD_ARG_TGT_ARCH`, `x64` gives `WIN64`, anything
else (or not set) gives `WIN32`.

| Environment | Default |
|-------------|---------|
| `vcvarsall.bat x64`, *x64 Native Tools Command Prompt*, fabricare `win64-*` | `WIN64` |
| `vcvarsall.bat x86`, *x86 Native Tools Command Prompt*, fabricare `win32-*` | `WIN32` |
| no Visual C++ environment variables | `WIN32` |

Older versions always defaulted to `WIN32`, so x64 libraries failed without
`--mode WIN64`. Scripts should still pass `--mode`: the result then does
not depend on the environment.

The mode must match the objects: with the wrong one `link` stops with
`LNK1112: module machine type 'x64' conflicts with target machine type
'x86'` and `lib-to-dll` exits `1`.

## Exit codes

| Code | Meaning |
|------|---------|
| `0` | `in.dll` made (`Build in.dll ok`); also no arguments, `--help`, `--license` / `--version` without a library |
| `1` | any error: bad option, no library, library not found, one of `lib`, `xyo-coff-to-def`, `link` failed, a file could not be moved or written |

## Messages

Every command is printed before it runs, the output of the tools follows.
`lib-to-dll` adds:

| Message | Cause |
|---------|-------|
| `Error: unknown option --x` | not one of the options above |
| `Error: --mode needs a value, WIN32 or WIN64` | `--mode` is the last argument |
| `Error: unknown mode X, use WIN32 or WIN64` | other value after `--mode` |
| `Error: no library specified` | options but no file name |
| `Error: in.lib not found` | neither `in.lib` nor `in.static.lib` exists |
| `Error: in.lib is an import library and in.static.lib not found` | `in.lib` is the import library of a DLL and the static library is gone |
| `Error: unable to list the members of in.lib` | `lib /list` failed (not a library, locked file); its output is printed before |
| `Error: in.static.lib has no members` | empty library |
| `Error: command failed, exit code N` | `lib`, `xyo-coff-to-def` or `link` failed; their own messages are above |
| `Error: unable to run the command, lib, link and xyo-coff-to-def must be on the PATH ...` | one of them could not be started |
| `Error: unable to remove ...`, `unable to move ...`, `unable to create ...`, `unable to write ...` | file in use, read only folder |
| `Warning: in.static.lib has more than one member named X, only the first one is used` | two objects with the same name in the library, see [Troubleshooting](troubleshooting.md#members-with-the-same-name) |

## Examples

x64 library, every symbol exported:

```
lib-to-dll --mode WIN64 zlib.lib
```

x86 library that needs another library:

```
lib-to-dll --mode WIN32 libpng.lib zlib.lib
```

Only the functions listed in `sqlite3.def` (next to `sqlite3.lib`), DLL
without the Visual C++ runtime DLLs (objects built with `/MT`):

```
lib-to-dll --mode WIN64 --static-crt sqlite3.lib
```

A path with spaces:

```
lib-to-dll --mode WIN64 "C:\Build Output\mylib.lib"
```
