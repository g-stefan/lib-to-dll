# Getting started

## 1. Requirements

- **Windows and Visual C++.** `lib-to-dll` runs `lib.exe` and `link.exe`;
  start it from a Visual C++ environment (a *Developer Command Prompt*,
  `vcvarsall.bat x64`, or a fabricare build, which sets it up itself).
- **`xyo-coff-to-def` on the `PATH`.** It writes the DEF file (the list of
  exported symbols) when there is no `in.def` next to the library, or with
  `--use-coff-def`. It is part of the XYO SDK, like `lib-to-dll`.
- **A static library compiled for the same machine** as the DLL to make:
  x86 objects for `--mode WIN32`, x64 objects for `--mode WIN64`. ARM64 is
  not supported.

## 2. Build and install

The tool is built with [fabricare](https://github.com/g-stefan/fabricare).
`xyo-platform`, `xyo-managed-memory`, `xyo-data-structures`,
`xyo-multithreading`, `xyo-encoding` and `xyo-system` must be installed to
the SDK first. From the repository root:

```bash
fabricare make       # build into output/
fabricare test       # convert a test library and use the DLL (needs make first)
fabricare install    # copy output/bin to ~/.fabricare/<platform>/bin
fabricare clean      # remove output/ and temp/
```

After `install`, `lib-to-dll` is in `~/.fabricare/<platform>/bin`, which
is on the `PATH` of a fabricare build, so the scripts of other projects can
call it.

## 3. First DLL

A static library `mylib.lib`, built with `/MD` (the default C runtime of
the DLL), x64:

```
lib-to-dll --mode WIN64 mylib.lib
```

Every command is printed before it runs; the last line is
`Build mylib.dll ok`, exit code `0`. In the folder of `mylib.lib`:

| File | What it is |
|------|------------|
| `mylib.dll` | the DLL, with all the objects of the library |
| `mylib.lib` | the **import library** of `mylib.dll` (it replaced the static one) |
| `mylib.static.lib` | the original static library |

In a x64 Visual C++ environment `--mode WIN64` can be left out: the default
is the target of the environment (see
[Command line](command-line.md#the-default-mode)).

If the library calls functions of other libraries, give them after it:

```
lib-to-dll --mode WIN64 mylib.lib zlib.lib ws2_32.lib
```

## 4. Choose what is exported

Without a DEF file every public symbol of every object is exported:
functions, global variables, C++ functions and methods (decorated names),
also inline functions and template instances that the objects contain.

To export only some functions, write `mylib.def` next to `mylib.lib`:

```
EXPORTS
	mylib_open
	mylib_read
	mylib_close
```

`lib-to-dll` then links with your DEF file (and leaves it in place).
`--use-coff-def` ignores it and exports everything again.

## 5. Use the DLL from a program

Link the program with `mylib.lib` as before; it is now the import library.
Copy `mylib.dll` next to the program (or into a folder on the `PATH`).

Functions work without changes in the headers. **Variables** need
`__declspec(dllimport)` in the declaration the program sees:

```c
// function: works with or without dllimport
int mylib_open(const char *name);

// variable exported by the DLL: dllimport is required
__declspec(dllimport) extern int mylib_errno;
```

Library headers that have an export macro usually switch it with a define,
for example `ZLIB_DLL` or `MYLIB_DLL`; define it in the program, not in the
library build. More in [Troubleshooting](troubleshooting.md#variables-data-exports).

## 6. In a fabricare script

```js
// make.js of a vendor-* project: build the static library, then the DLL
exitIf(Shell.system("lib-to-dll --mode WIN64 output\\lib\\mylib.lib"));
```

`Shell.system` returns the exit code of `lib-to-dll`, so `exitIf` stops the
build when the conversion fails. Pick the mode from the platform when the
script builds for both:

```js
var mode = (Platform.osType == "win64") ? "WIN64" : "WIN32";
exitIf(Shell.system("lib-to-dll --mode " + mode + " output\\lib\\mylib.lib"));
```

## 7. Build the library again

Run `lib-to-dll` again after the build writes a new static `mylib.lib`: it
sees that `mylib.lib` is a static library (not the import library of a
previous run), replaces `mylib.static.lib` with it and makes a new DLL.
Running it again without a new build uses `mylib.static.lib` and gives the
same DLL. See [How it works](how-it-works.md#converting-again).
