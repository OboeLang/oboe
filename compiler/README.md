A transpiler from Oboe to C, plus the `oboe` CLI described in SPEC.md.

## Layout

```
selfhost/   the compiler, written in Oboe        -> bin/oboec
src/        the CLI and package manager, in C    -> bin/oboe
runtime/    oboe_runtime.{c,h}, embedded into every compiled program
bootstrap/  generated: the C that bin/oboec is built from
tests/      the suite (see below), driven by tests/run_tests.sh
```

## Build

```
make
```

Produces `bin/oboe` (the CLI, in C) and `bin/oboec` (the compiler proper, written in Oboe). Run the test suite with `make test`: each `tests/*.oboe` is run and diffed against its `.expected` output; `fail_*` tests assert that compilation/execution fails with the message in their `.expect_fail` file, and `_*.oboe` files are helper modules imported by other tests, not run directly.

The suite does not test `bin/oboec` itself. Instead the suite first uses `bin/oboec` to build a compiler from the current `selfhost/` sources into `stage/suite/`, and runs everything against that.

## Self-hosting

`selfhost/` is the compiler proper, written in Oboe, transpiling to C and handing the C to gcc. Only the compiler proper moved; the CLI, the package manager, the katare client and the vendored wire-format code are still in C for now, but I'd eventually like to move them to Oboe.

Notes about miscellaneous/old/irrelevant crap:

- `selfhost/{lexer,parser,codegen,dump,docs,diag,hostos,main}.oboe`: `oboec`. `bin/oboe` execs it for every `run`, `build`, `install` and `doc`. 
- `selfhost/mini/` is a compiler for a small Oboe-like subset, written in Oboe. It was the spike that proved the language could host a compiler at all, and it stays as a cheap regression test engine for the features that made that possible (break/continue, short-circuit `and`/`or`, `ord`, dict-shaped AST nodes, `eprint` + `os.exit`).

### Bootstrap

`bin/oboec` is built from `bootstrap/oboec.c`, a committed generated file, so a clean checkout needs nothing but a C compiler. Refreshing it is `make regen-bootstrap` -- deliberately a separate step, since it is a reviewed change rather than part of the build.

That target also has to scrub two facts about the machine that generated the file, since every other machine then builds it unchanged:

- The three `os.script_*` constants are blanked, because a committed artifact must not carry the absolute paths of whoever generated it. Safe only because `oboec` never calls them.
- `HOST_OS` becomes a preprocessor conditional (`bootstrap/hostos.h`, prepended to the generated C). `selfhost/hostos.oboe` is a per-OS module, so the OS is chosen at generation time; left alone, a `bin/oboec` built on, say, macOS from a Linux-generated bootstrap resolves `foo.linux.oboe` for every import. `bin/oboe` passes `--target-os` on every invocation regardless, so this is the default only for a bare `oboec`.

Both substitutions are verified by `regen-bootstrap` and pinned by the suite (`bootstrap_host_os_not_baked`, `bootstrap_host_os_follows_compiler`).

`make bootstrap-check` is the proof that the compiler reproduces itself: stage1 is the committed bootstrap, stage2 is what it emits from today's `selfhost/` sources, stage3 is what a compiler built from stage2 emits from the same sources, and stage2 must equal stage3. stage1 may legitimately differ from both, having been generated before whatever change is under test.

### Front-end snapshots

`oboec --dump-tokens <file>` and `oboec --dump-ast <file>` print a file's tokens and syntax tree, for debugging the compiler. The suite also uses them to check stuff the golden tests don't:

- `snapshot_lexer`: the tokens of `tests/helpers/lexer_torture*.oboe` and of a deliberately bad character.
- `snapshot_parser`: the tree of `tests/helpers/parser_torture.oboe`, plus the diagnostic for every malformed input in `tests/helpers/parse_errors.txt`, which between them reach every reachable parse error.
- `snapshot_codegen_errors`: the diagnostic for every input in `tests/helpers/codegen_errors.txt`, which reach every codegen error the `fail_*` tests don't.

`tests/helpers/parser_torture.oboe` tests the grammar corners. Much of it is deliberately strange, in particular a block of expressions split so that each operator sits on its own line, which is the only way a wrong line-capture point in the parser becomes visible. `tests/codegen_corners.oboe` does the same for emission orderings that only differ when a program is shaped a particular way. For example, a class declared before its parent, an exact-count constructor overload behind one whose defaults also fit, a string that looks like an import.

`oboec` gets the OS it targets by default from `selfhost/hostos.oboe`, which is a per-OS module file rather than a constant: that is the mechanism the language already has for this, and `hostos.oboe`/`hostos.macos.oboe`/`hostos.windows.oboe` are the three arms of the `#if` in `bootstrap/hostos.h`.

## Usage

```
bin/oboe init [dir]                     # scaffold a project here, or in a new/existing directory
bin/oboe run path/to/file.oboe [args]   # anything after the file is the program's
bin/oboe run                            # runs the project's entry file (project.jsonc)
bin/oboe run -- [args]                  # the same, with arguments for the program
bin/oboe build [file] [-o out] [-v]     # project -> dist/<name>; file -> ./<file-stem>
    [-t <config-or-os>] [--cc compiler]        # a project.jsonc build.targets name, or
                                               # linux|windows|macos|freebsd|openbsd|netbsd
    [--emit-c]                                 # write the generated C, don't compile it
    [--desktop] [--meta-name N] [--meta-version V] [--meta-description D] [--meta-icon I]
bin/oboe tidy [-v]                      # installs what project.jsonc declares; no-op outside a project
bin/oboe remove <pkg>                   # deletes a package's files and dependency entry
bin/oboe get <pkg>[@<constraint>]       # fetch a library into .oboe/libraries
bin/oboe install <pkg>[@<constraint>]   # fetch, build and place a tool in $OBOE_HOME/bin
bin/oboe publish [--dry-run]            # pack this project and upload it
bin/oboe doc <file>                     # the file's docstrings as markdown, on stdout
bin/oboe sema <file>...                 # sha256 in the wire form; `-` reads stdin
    [--registry katare://host[:port]/]  # else $OBOE_REGISTRY, else project.jsonc, else built-in
bin/oboe --version                      # `oboe <version> (<host os>)`; -V is the same
bin/oboe --help                         # the usage above; -h is the same
```

## Packages

Packages come from a katare registry. The protocol and the reference server live in the [reedbed](https://github.com/OboeLang/reedbed) repository. The wire format code (`sha256`, `izim`, `record`, `kabuk`, `projectjson`) is vendored from reedbed's `common/` and checked against `src/VENDOR.sha256` by the test suite. Don't edit that code here, edit it there and re-vendor.

`oboe get` resolves a version set, fetches each archive, verifies its SHA-256 while streaming, and unpacks it into `.oboe/libraries/<name>/`. The exact versions land in `.oboe/lock`, which is meant to be committed; `oboe tidy` honours it and makes no connection at all when everything it names is already present.

`oboe publish` packs the project into a kabuk: uncompressed, sorted by byte order, no modes or timestamps, so the same tree always produces the same octets and therefore the same digest. `dist/`, `.git/`, `.oboe/`, `*.o` and editor droppings are always excluded; `.oboeignore` handles the rest. A symlink is refused rather than followed, because the format cannot represent one.

`oboe build` with a `build.targets` object in project.jsonc builds every declared target in turn, each with its own settings layered under the CLI flags; `-t <name>` picks one. Without `targets` it builds once, as before. `oboe build <file>` is always a single script.

## Design

Every Oboe value is represented at runtime as a single dynamic, tagged `OboeValue` (see `runtime/oboe_runtime.h`), rather than mapped to native C types per declared type. This sidesteps needing a full static type checker in this first pass, and gives heterogeneous arrays/dicts, `??`/`?.`, and `is` type-checks for free. Type annotations in Oboe source are otherwise unenforced at compile time in this version, except for one place: resolving which class's fields/methods a `.member` access refers to, which requires the compiler to know an expression's clas* statically. A lightweight local pass tracks class types only (not primitive types) through `let`, parameters, field declarations, and a function's or method's declared return type; a `.method()`/`.field` access on an expression whose class can't be inferred is a compile error.

A string's bytes are preceded in the same allocation by a header, with `as.s` pointing past it, so `ob_slen()` is O(1) while the payload stays an ordinary NUL-terminated C string that `strcmp`, `printf` and the FFI can use unchanged. Without that, walking a string a character at a time is quadratic -- `.substr()` needs the subject's length just to clamp, and `strlen` re-derives it every call. `ob_string()` and `ob_string_take()` are the only two functions that ever fill in `as.s`; the header only exists in front of pointers they produced, so `ob_slen()` must never be handed an interior pointer.

The header carries the byte length, and then -- counted lazily on first use, so a string that is only concatenated or printed never pays for it -- the codepoint length, whether the payload is well-formed UTF-8, and a one-entry cursor memoizing the last codepoint index resolved to a byte offset. Language-level string indices are codepoints, so `ob_cp_to_byte()` sits under `.substr()`, `.slice()` and iteration, and the cursor is what keeps it O(1) for the access patterns that actually occur. It walks from whichever of the start, the end or the cursor is nearest.

Stepping *backwards* is only available for well-formed payloads. Normally a codepoint boundary is just the first byte at or below the target that isn't a continuation byte, but the total decoding above breaks that inference: given `C3 A9`, the `A9` is a boundary if the `C3` was a truncated lead and isn't if the two are a well-formed `é`, and nothing local to the `A9` distinguishes the cases. Ill-formed payloads therefore only walk forwards, restarting from the beginning when the target is behind the cursor. That distinction is worth the flag it costs: a lexer reads `byte_at(POS + 1)` and then `byte_at(POS)`, so forward-only walking turns every lookahead into a rescan from the start of the file -- measured at 89s versus 0.5s to lex 12k lines, once a single non-ASCII byte anywhere in the file takes the all-ASCII fast path away.

Classes compile to plain C structs, with a class's parent embedded as the struct's first member (standard-layout pointer-cast trick), so inherited methods/fields are reachable via a pointer cast rather than a vtable, as the spec specifies non-virtual dispatch. `super(...)` and `super.method()` compile to direct calls on the nearest ancestor that defines the constructor/method, and a class with no `init` gets thin wrappers around its ancestor's constructors. Instance fields aren't declared up front; the compiler infers a class's field set by scanning all of its methods for `this.field = ...` assignments. `static`/`const` fields still use an explicit declaration (`static int count = 0`), since they aren't tied to any particular instance.

`try`/`catch`/`finally` is implemented with `setjmp`/`longjmp`; exceptions are matched by type name string, most-specific catch clause first.

Compilation runs over a unit list: the main file plus every transitively imported module. All sources are loaded (via a lightweight textual scan for `import` lines) and pre-scanned for `operator <sym>` declarations before anything is tokenized, then parsed, then the real import graph is resolved from the ASTs and code is generated in one pass. Import aliases and `from` bindings are scoped to the importing file; bare function calls resolve within their own unit.

Operator overloading is dynamic: each binary operator call first checks whether the left-hand object's class (or an ancestor) registered a handler, falling back to the builtin behavior otherwise. Class overloads use the spec's `operator + (this, Vector2 other) { ... }` syntax and register at startup. Top-level `operator ||> (int a, int b) { ... }` declarations introduce new operator tokens; because symbols are registered before lexing (by a text pre-scan that skips comments and string literals), an operator declared in any file (including an imported module) is usable everywhere. A symbol is built from `+-*/%<>=!&|^~?@#$:.`, with `_` also allowed after the first character (`^_^`). A word works too (`operator foo (a, b)`): like the builtin `x`, it is still lexed as an identifier and only read as an operator in infix position, and only when it is on the same line as its left operand, since semicolons are optional and a line starting with `foo(...)` has to stay a call. A top-level declaration of a *builtin* operator is an error, because builtin operators are only overloaded per class. Custom operators bind tighter than `and`/`or` and looser than the bitwise operators, left-associative.

The events system compiles each `event E = event(...)` to a struct (payload fields become members) plus an `E__fire` function that constructs the payload object and calls every `on E` handler in declaration order -- dispatch is fully static, no runtime handler list. `on KeyboardInterruptEvent` installs a SIGINT handler with the spec's semantics: the first ^C runs the handlers and exits; a second ^C while they run exits immediately.

FFI uses `cimport symbol from "library.so"`. The symbol is resolved with `dlopen`/`dlsym` at startup and called through a word-sized shim: ints, bools, and nulls pass by value, strings pass as C string pointers, at most 8 arguments, and the return value comes back as an Oboe int.

## Known limitations of this first pass

- No static type checking beyond class-type tracking described above. Primitive type annotations are accepted but not enforced, with one exception: the numeric types (`int8`..`uint64`, `float32`/`float64`) are enforced at stores, where the runtime wraps or rounds the value to the declared type. A mismatched type otherwise surfaces only as a runtime error.
- Generally, we make a best-effort to provide compile errors, but officially, the entirety of invalid Oboe is UB.
- Constructor overload resolution is by argument count only, not by type.
- `import` resolves a module by looking for `<module>.<target-os>.oboe`, then `<module>.oboe`, then a module *folder* -- first next to the importing file, then under that file's `.oboe/libraries/`, and finally under the *project's* `.oboe/libraries/` -- and inlines it into the same translation unit with name-prefixing. That last root is what lets an installed package import another installed package, since its dependencies are its siblings there rather than anything beneath it. A folder with a project.jsonc is imported under that project's `name` and entered through its `entry`; one without is imported under its directory name and entered through `main.oboe`. Each unit resolves its own imports relative to itself, so a library folder can import its siblings by bare name. `math`, `random` and `os` fall back to runtime built-ins when no file shadows them.
- Only one version of a package can be installed at a time: `.oboe/libraries/` is a flat namespace, so a diamond dependency whose constraints cannot be satisfied by a single version is an error rather than a nested install.
- `oboe build` produces a single executable per target (no separate .so/DLL embedding yet), plus a `.desktop` file or `.app` bundle beside it under `--desktop`. Cross-compiling relies on an installed mingw-w64 (`-t windows`), osxcross (`-t macos`) or an appropriate clang for the BSD targets; Windows version metadata is embedded only when `x86_64-w64-mingw32-windres` is available, and macOS metadata lives in the `.app` bundle's Info.plist.
- `project.jsonc` (or, as a fallback, `project.json`) is read with a minimal targeted scan for the fields this toolchain needs, not a general JSON parser (see `src/projectjson.c`). Lookups against a nested object are depth-aware so a setting in `build.targets.<name>` can't answer a lookup against `build`.
- Methods on primitives (`"a,b".split(",")`, `[1,2].len()`) dispatch on the receiver's tag at runtime, since the compiler never knows a primitive's type statically; a wrong receiver throws a catchable `TypeError` rather than failing to compile. A user class's own method always wins over a builtin of the same name.
- Default and named arguments are bound at the call site against the target's declaration, so the generated C stays positional. Two consequences: `f(x = 1)` is always a by-name argument, never an assignment expression passed by value; and a default expression is evaluated in the *caller's* scope, so it can name module-level state but not the function's own earlier parameters.
- Strings are codepoint-oriented: `.len()` and every string index count characters, and `for (c in s)`, `.reverse()` and `.split("")` never split a multi-byte one. Three limits remain.  See SPEC.md for more.
- `let` is still accepted as a legacy alias for `var`; the spec spelling (`var x`, `int x`, `const var x`, `const int x`) is preferred.
- Events and operators are global once their file is part of the program; `on` handlers fire in collection order (main file's first). An event's *name* is therefore global and unqualified, but a handler may still write `on <module>.<event>`: the qualifier resolves through the handling file's own import bindings and is checked against the unit that declared the event, so it documents (and enforces) where the event came from rather than selecting between same-named events, which cannot coexist. Firing someone else's event from outside its module still needs `import <event> from <module>`, since `<module>.<event>.fire(...)` is a member access on a value whose class the compiler can't infer.
- Module top-level statements run once at startup in load order (deepest import first); that order is a good approximation of dependency order but isn't a true topological sort for diamond-shaped import graphs.
- FFI arguments/returns are limited to word-sized values (ints, bools, C strings); no floats, structs, or out-parameters.
