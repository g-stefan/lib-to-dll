# Troubleshooting

## LNK1112: module machine type 'x64' conflicts with target machine type 'x86'

The mode does not match the objects: here x64 objects with `--mode WIN32`
(or no `--mode` outside a x64 Visual C++ environment). `link` stops,
`lib-to-dll` exits `1`. Run again with `--mode WIN64` (x64 objects) or
`--mode WIN32` (x86 objects). `dumpbin /headers in.static.lib` shows the
machine of the objects.

## LNK2019 / LNK2001: unresolved external symbol

A DLL must be complete: every function the library calls must be in the
library or in a library given to `link`. The C runtime and `kernel32.lib`
come by default; other libraries do not. Give them after the library:

```
lib-to-dll --mode WIN64 libpng.lib zlib.lib
lib-to-dll --mode WIN64 mynet.lib ws2_32.lib advapi32.lib
```

The symbol name in the message tells which library is missing
(`__imp_send` → `ws2_32.lib`, `__imp_RegOpenKeyExW` → `advapi32.lib`).

## LNK2005: already defined / LNK4098: defaultlib conflicts

The objects and the C runtime do not match. Objects built with `/MD` (or
`/MDd`) go with the default; objects built with `/MT` (or `/MTd`) need
`--static-crt`. Debug objects (`/MDd`, `/MTd`) need the debug runtime,
which `lib-to-dll` does not select: build the library in release mode, or
link it yourself.

## Variables (data exports)

A variable exported by a DLL is reached through a pointer in the import
library (`__imp_name`). Code that uses it must declare it
`__declspec(dllimport)`:

```c
__declspec(dllimport) extern int mylib_errno;
```

Without `dllimport` the program fails to link (`unresolved external
symbol mylib_errno`), or, with old `xyo-coff-to-def` versions that did not
mark variables `DATA`, it links and reads the wrong memory. Functions work
with or without `dllimport`.

## Objects compiled with /GL (whole program optimization)

Objects compiled with `/GL` hold intermediate code, not COFF symbols:
`xyo-coff-to-def` can not read them and the run stops. Build the library
without `/GL` (and without `/LTCG` for the library), or write `in.def` by
hand.

## Members with the same name

`Warning: in.static.lib has more than one member named X, only the first
one is used`

`lib /extract` takes a member by name, so two objects stored with the same
name (for example `util.obj` from two folders, added with the same
relative path) can not both be extracted. The second one is left out, and
its functions are missing from the DLL (`unresolved external` while
linking, or missing exports). Rename one of the source files, or build the
library with objects in different folders so their stored names differ.

## Only some functions should be exported

Write `in.def` next to `in.lib`:

```
EXPORTS
	mylib_open
	mylib_close
```

C names without the x86 `_` prefix (`link` adds it); C++ names decorated
as the compiler writes them (`dumpbin /symbols in.static.lib` lists them).
Do not pass `--use-coff-def`, it ignores `in.def`.

## The DLL exports too much

Without `in.def` every external symbol of the objects is exported,
including inline functions and template instances from headers (for
example `std::` functions). It is harmless; write `in.def` if a clean
export list matters.

## Error: in.lib is an import library and in.static.lib not found

`in.lib` is the import library of a DLL (from an earlier conversion, or a
library that already ships as a DLL) and the static library is gone.
Build the static library again, or restore `in.static.lib`.

## Error: unable to run the command, lib, link and xyo-coff-to-def must be on the PATH

`lib`, `link` or `xyo-coff-to-def` could not be started. Open a Visual C++
environment (`vcvarsall.bat x64`, or run from a fabricare build) and check
that the XYO SDK `bin` folder (`xyo-coff-to-def`) is on the `PATH`.

## Limits

- Windows and Visual C++ only; `WIN32` (x86) and `WIN64` (x64) only, no
  ARM64.
- Release C runtime only (`msvcrt` / `libcmt`), see above.
- No resources, version information or manifest in the DLL: link it
  yourself (`link /dll ... in.res`) if they are needed.
- The `xyo-coff-to-def` command line is limited to 32767 characters: very
  large libraries with long member names can exceed it; write `in.def`
  then, or extract and run `xyo-coff-to-def` with a response file.
