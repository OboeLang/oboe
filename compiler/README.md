A transpiler from Oboe to C, plus the `oboe` CLI described in SPEC.md.

## Build

```
make
```

Produces `bin/oboe`. Run the test suite with `make test`: each `tests/*.oboe`
is run and diffed against its `.expected` output; `fail_*` tests assert that
compilation/execution fails with the message in their `.expect_fail` file, and
`_*.oboe` files are helper modules imported by other tests, not run directly.

## Usage

```
bin/oboe init [dir]                     # scaffold a project here, or in a new/existing directory
bin/oboe run path/to/file.oboe
bin/oboe run                            # runs the project's entry file (project.json)
bin/oboe build [file] [-o out] [-v]     # project -> dist/<name>; file -> ./<file-stem>
    [-t <config-or-os>] [--cc compiler]        # a project.json build.targets name, or
                                               # linux|windows|macos|freebsd|openbsd|netbsd
    [--desktop] [--meta-name N] [--meta-version V] [--meta-description D] [--meta-icon I]
bin/oboe tidy [-v]                      # no-op outside a project directory
bin/oboe remove <pkg>                   # deletes a package's files and dependency entry
bin/oboe get <pkg> / oboe install <pkg> # stubs; no package repository exists yet
```

`oboe build` with a `build.targets` object in project.json builds every declared target
in turn, each with its own settings layered under the CLI flags; `-t <name>` picks one.
Without `targets` it builds once, as before. `oboe build <file>` is always a single
script, never a target name.

## Design

Every Oboe value is represented at runtime as a single dynamic, tagged `OboeValue` (see `runtime/oboe_runtime.h`), rather than mapped to native C types per declared type. This sidesteps needing a full static type checker in this first pass, and gives heterogeneous arrays/dicts, `??`/`?.`, and `is` type-checks for free. Type annotations in Oboe source are otherwise unenforced at compile time in this version, except for one place: resolving which class's fields/methods a `.member` access refers to, which requires the compiler to know an expression's clas* statically. A lightweight local pass tracks class types only (not primitive types) through `let`, parameters, and field declarations; a `.method()`/`.field` access on an expression whose class can't be inferred is a compile error.

Classes compile to plain C structs, with a class's parent embedded as the struct's first member (standard-layout pointer-cast trick), so inherited methods/fields are reachable via a pointer cast rather than a vtable, as the spec specifies non-virtual dispatch. `super(...)` and `super.method()` compile to direct calls on the nearest ancestor that defines the constructor/method, and a class with no `init` gets thin wrappers around its ancestor's constructors. Instance fields aren't declared up front; the compiler infers a class's field set by scanning all of its methods for `this.field = ...` assignments. `static`/`const` fields still use an explicit declaration (`static int count = 0`), since they aren't tied to any particular instance.

`try`/`catch`/`finally` is implemented with `setjmp`/`longjmp`; exceptions are matched by type name string, most-specific catch clause first.

Compilation runs over a unit list: the main file plus every transitively imported module. All sources are loaded (via a lightweight textual scan for `import` lines) and pre-scanned for `operator <sym>` declarations before anything is tokenized, then parsed, then the real import graph is resolved from the ASTs and code is generated in one pass. Import aliases and `from` bindings are scoped to the importing file; bare function calls resolve within their own unit.

Operator overloading is dynamic: each binary operator call first checks whether the left-hand object's class (or an ancestor) registered a handler, falling back to the builtin behavior otherwise. Class overloads use the spec's `operator + (this, Vector2 other) { ... }` syntax and register at startup. Top-level `operator ||> (int a, int b) { ... }` declarations introduce new operator tokens; because symbols are registered before lexing, an operator declared in any file (including an imported module) is usable everywhere. Custom operators bind tighter than `and`/`or` and looser than comparisons, left-associative.

The events system compiles each `event E = event(...)` to a struct (payload fields become members) plus an `E__fire` function that constructs the payload object and calls every `on E` handler in declaration order -- dispatch is fully static, no runtime handler list. `on KeyboardInterruptEvent` installs a SIGINT handler with the spec's semantics: the first ^C runs the handlers and exits; a second ^C while they run exits immediately.

FFI uses `cimport symbol from "library.so"`. The symbol is resolved with `dlopen`/`dlsym` at startup and called through a word-sized shim: ints, bools, and nulls pass by value, strings pass as C string pointers, at most 8 arguments, and the return value comes back as an Oboe int.

## Known limitations of this first pass

- No static type checking beyond class-type tracking described above. Primitive type annotations are accepted but not enforced, with one exception: the numeric types (`int8`..`uint64`, `float32`/`float64`) are enforced at stores, where the runtime wraps or rounds the value to the declared type. A mismatched type otherwise surfaces only as a runtime error.
- Generally, we make a best-effort to provide compile errors, but officially, the entirety of invalid Oboe is UB.
- Constructor overload resolution is by argument count only, not by type.
- `import` resolves a module by looking for `<module>.<target-os>.oboe`, then `<module>.oboe`, then a module *folder*, first next to the importing file and then under `.oboe/libraries/`, and inlines it into the same translation unit with name-prefixing. A folder with a project.json is imported under that project's `name` and entered through its `entry`; one without is imported under its directory name and entered through `main.oboe`. Each unit resolves its own imports relative to itself, so a library folder can import its siblings by bare name. `math`, `random` and `os` fall back to runtime built-ins when no file shadows them. There's no real package repository yet, so `oboe get`/`oboe install` are stubs -- but `oboe remove` genuinely deletes a package's files and dependency entry.
- `oboe build` produces a single executable per target (no separate .so/DLL embedding yet), plus a `.desktop` file or `.app` bundle beside it under `--desktop`. Cross-compiling relies on an installed mingw-w64 (`-t windows`), osxcross (`-t macos`) or an appropriate clang for the BSD targets; Windows version metadata is embedded only when `x86_64-w64-mingw32-windres` is available, and macOS metadata lives in the `.app` bundle's Info.plist.
- `project.json` is read with a minimal targeted scan for the fields this toolchain needs, not a general JSON parser (see `src/projectjson.c`). Lookups against a nested object are depth-aware so a setting in `build.targets.<name>` can't answer a lookup against `build`.
- Methods on primitives (`"a,b".split(",")`, `[1,2].len()`) dispatch on the receiver's tag at runtime, since the compiler never knows a primitive's type statically; a wrong receiver throws a catchable `TypeError` rather than failing to compile. A user class's own method always wins over a builtin of the same name.
- Default and named arguments are bound at the call site against the target's declaration, so the generated C stays positional. Two consequences: `f(x = 1)` is always a by-name argument, never an assignment expression passed by value; and a default expression is evaluated in the *caller's* scope, so it can name module-level state but not the function's own earlier parameters.
- Strings are byte-oriented throughout: `for (c in s)`, `.len()` and `.reverse()` all work a byte at a time, so multi-byte UTF-8 characters are split.
- `let` is still accepted as a legacy alias for `var`; the spec spelling (`var x`, `int x`, `const var x`, `const int x`) is preferred.
- Events and operators are global once their file is part of the program; `on` handlers fire in collection order (main file's first).
- Module top-level statements run once at startup in load order (deepest import first); that order is a good approximation of dependency order but isn't a true topological sort for diamond-shaped import graphs.
- FFI arguments/returns are limited to word-sized values (ints, bools, C strings); no floats, structs, or out-parameters.
