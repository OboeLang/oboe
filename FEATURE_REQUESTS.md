# Open questions for features to be added

# Perhaps sometime far in the future

- Add C++ support to `cimport`?
    - Separate syntax or the same?

# Unimplemented feature requests

- Local/global variables?
- Syntax for choosing a C file to import based on OS (specifically the `foo.windows.oboe` thing)
- Inheritance: syntax for declaring a parent, constructor chaining, and override rules.
    - `extends` and `super()`
- More build targets than just those two/three?
- More options for the build command to set metadata that is mostly used on Windows and MacOS
- Build option to generate a desktop file, perhaps using above metadata
- Ability to choose to set these build flags in the project json
- Build target command for `oboe build`
    - Like `-t` / `--target`
    - `nt`, `darwin` and `linux` (actually, should it just be `nt` and `linux`?)
    - optionally, replace `nt` and `darwin` with `windows` and `macos`
- If/Else shorthand (tenary operators)
- Various features in namespaced parts of the stdlib
    - Command for running commands to the terminal running the process
    - Command for spawning a process
        - See above
    - Some built in math functions
    - Some built in random functions
    - File operation methods