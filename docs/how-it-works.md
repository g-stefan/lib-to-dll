# How it works

`lib-to-dll in.lib extra.lib` with `--mode WIN64`, step by step. Every
command is printed before it runs. External commands are started directly
(not through `cmd.exe`), so paths with spaces only need quotes, and a
command line can be up to 32767 characters.

## 1. Pick the static library

`in.lib` is listed with `lib /nologo /list in.lib`.

| `in.lib` | `in.static.lib` | What happens |
|----------|-----------------|--------------|
| static library | missing | `in.lib` is moved to `in.static.lib` (first conversion) |
| static library | exists | `in.static.lib` is removed, `in.lib` is moved to `in.static.lib` (the library was built again) |
| import library | exists | `in.static.lib` is used (converted before, nothing new) |
| import library | missing | `Error: in.lib is an import library and in.static.lib not found` |
| missing | exists | `in.static.lib` is used |
| missing | missing | `Error: in.lib not found` |

`in.lib` is an **import library** when every member is named after a DLL
or an executable (`in.dll`, `other.exe`): that is how `link.exe` and
`lib.exe` name the members of import libraries. A static library has
object members (`adler32.obj`, `x64\Release\inflate.obj`).

The static library is moved, not copied, so it is never lost: if a later
step fails, it is in `in.static.lib` and the next run uses it.

## 2. Extract the objects

`lib /nologo /list in.static.lib` gives the member names, written to
`in.lst`. The folder `in.obj\` is created empty and each member is
extracted into it:

```
lib /nologo /MACHINE:X64 /extract:x64\Release\adler32.obj /out:in.obj\x64_Release_adler32.obj in.static.lib
```

The file name in `in.obj\` is the member name with `.\` removed, then `_`
doubled, then `\`, `/` and `:` replaced by `_`, so members from different
folders do not overwrite each other (`a\b.obj` → `a_b.obj`, `a_b.obj` →
`a__b.obj`).

## 3. The DEF file

| Situation | DEF file given to `link` |
|-----------|--------------------------|
| `in.def` exists, no `--use-coff-def` | `in.def`, as it is |
| no `in.def`, or `--use-coff-def` | `in.dll.def`, written by `xyo-coff-to-def` |

```
xyo-coff-to-def --out in.dll.def --mode WIN64 in.obj\x64_Release_adler32.obj ...
```

`xyo-coff-to-def` reads the COFF symbol table of every object and lists
every external symbol the object defines (functions and variables, C and
C++ names), minus the compiler's internal symbols. In `WIN32` mode it
removes the `_` that the x86 C compiler puts before C names (`_adler32` →
`adler32`); `link.exe` adds it back. The exact rules are in the
[xyo-coff-to-def documentation](https://github.com/g-stefan/xyo-coff-to-def).

## 4. Link the DLL

The options and the file names go into the response file `in.rsp`, one
object per line, so the number of objects is not limited:

```
/NOLOGO /OUT:in.dll /MACHINE:X64 /nodefaultlib:libcmt /defaultlib:msvcrt /dll /INCREMENTAL:NO /DEF:in.dll.def /implib:in.lib
in.obj\x64_Release_adler32.obj
...
extra.lib
```

```
link @in.rsp
```

`link.exe` writes `in.dll`, the import library `in.lib` and `in.exp`.
`/implib:in.lib` is why programs keep linking the same file name.

| Option | Default (`/MD` objects) | `--static-crt` (`/MT` objects) |
|--------|-------------------------|--------------------------------|
| C runtime | `/nodefaultlib:libcmt /defaultlib:msvcrt` | `/nodefaultlib:msvcrt /defaultlib:libcmt` |
| DLL depends on | `VCRUNTIME140.dll`, `api-ms-win-crt-*.dll` | only Windows DLLs |

The DLL entry point is the C runtime's (`_DllMainCRTStartup`), which runs
the constructors of the C++ global objects; the library does not need a
`DllMain`.

## 5. Clean up

`in.obj\`, `in.lst`, `in.rsp`, `in.exp` and `in.dll.def` (only the
generated one, never your `in.def`) are removed, after success and after
an error. On success the last line is `Build in.dll ok`.

## Converting again

| Run | Result |
|-----|--------|
| `lib-to-dll in.lib` twice, no build between | the same DLL, made again from `in.static.lib` |
| build writes a new static `in.lib`, then `lib-to-dll in.lib` | the new `in.lib` replaces `in.static.lib`, new DLL |
| a run failed (link error, wrong mode) | `in.static.lib` holds the library, `in.lib` may be missing or a leftover import library; fix the cause and run again |

Older versions always used `in.static.lib` when it existed, so after a new
build they made the DLL from the old code and overwrote the new static
library with the import library.

## Files

| File | Written by | Kept |
|------|------------|------|
| `in.static.lib` | step 1 (moved from `in.lib`) | yes |
| `in.dll` | `link` | yes |
| `in.lib` | `link` (`/implib`) | yes, import library |
| `in.def` | you | yes, never changed |
| `in.lst` | `lib /list` | no |
| `in.obj\` | `lib /extract` | no |
| `in.dll.def` | `xyo-coff-to-def` | no |
| `in.rsp` | `lib-to-dll` | no |
| `in.exp` | `link` | no |
