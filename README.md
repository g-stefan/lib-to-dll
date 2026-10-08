# Lib to Dll

Convert lib to dll
- Makes a DLL from a static library (`.lib`) built with Visual C++,
without changing the code or the build of the library.
- Keeps the static library as `in.static.lib` and writes the import
library as `in.lib`, so programs keep linking the same file name.
- Exports every public symbol of the objects (`xyo-coff-to-def`), or only
the ones of your `in.def`; extra libraries linked into the DLL;
`--static-crt` for a DLL without the Visual C++ runtime DLLs.

```
lib-to-dll --mode WIN64 mylib.lib
```

Runs `lib.exe`, `link.exe` and `xyo-coff-to-def` (Windows, Visual C++).

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - requirements, build, first DLL, use the DLL, fabricare scripts
- [Command line](docs/command-line.md) - options, the default mode, exit codes, messages, examples
- [How it works](docs/how-it-works.md) - every step, the files, converting again, DEF file, C runtime
- [Troubleshooting](docs/troubleshooting.md) - link errors, machine mismatch, `/GL`, data exports, limits

A Claude Code skill for this tool is in
[.claude/skills/lib-to-dll](.claude/skills/lib-to-dll/SKILL.md).

## License

Copyright (c) 2014-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
