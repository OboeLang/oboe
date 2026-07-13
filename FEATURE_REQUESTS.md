# Open questions for features to be added

- Rust-style `?` error-propagation operator: whether it fits Oboe's error-handling model.
- Further domain-specific operators
- Functional-pattern constructs (LINQ-style query syntax is of interest, not yet designed).
- Lambda functions or whatever they're called in JS? (the () => {} thing)
- If/Else shorthand? (tenary operators)
- Command for running commands to the terminal running the process?
    - Or should this just be in the OS library?
- Command for spawning a process?
    - See above
- Some built in math functions?
- Some build in random functions?
- Build target command for `oboe build`
    - Like `-t` / `--target`
    - `nt`, `darwin` and `linux` (actually, should it just be `nt` and `linux`?)
    - optionally, replace `nt` and `darwin` with `windows` and `macos`
- Easy building the CLI to above targets instead of just Linux
- More build targets than just those two/three?
- More options for the build command to set metadata that is mostly used on Windows and MacOS
- Build option to generate a desktop file, perhaps using above metadata
- Ability to choose to set these build flags in the project json
- Syntax for choosing a C file to import based on OS
- File operation methods?
    - Or should this just be in the OS library
- Add C++ support to `cimport`?
    - Separate syntax or the same?
- Local/global variables?

# Approved but uncompleted syntax drafts

- Inheritance: syntax for declaring a parent, constructor chaining, and override rules.

# Unimplemented syntax drafts

# Approved non-syntax feature requests, unimplemented