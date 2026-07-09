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
bin/oboe init                           # scaffold a project in the current directory
bin/oboe run path/to/file.oboe
bin/oboe run                            # runs the project's entry file (project.json)
bin/oboe build [--consolidate|-c]
bin/oboe tidy
bin/oboe get <pkg> / oboe install <pkg> # stubs; no package repository exists yet
```

## Design

Every Oboe value is represented at runtime as a single dynamic, tagged `OboeValue` (see `runtime/oboe_runtime.h`), rather than mapped to native C types per declared type. This sidesteps needing a full static type checker in this first pass, and gives heterogeneous arrays/dicts, `??`/`?.`, and `is` type-checks for free. Type annotations in Oboe source are otherwise unenforced at compile time in this version, except for one place: resolving which class's fields/methods a `.member` access refers to, which requires the compiler to know an expression's clas* statically. A lightweight local pass tracks class types only (not primitive types) through `let`, parameters, and field declarations; a `.method()`/`.field` access on an expression whose class can't be inferred is a compile error.

Classes compile to plain C structs, with a class's parent embedded as the struct's first member (standard-layout pointer-cast trick), so inherited methods/fields are reachable via a pointer cast rather than a vtable, as the spec specifies non-virtual dispatch. Instance fields aren't declared up front; the compiler infers a class's field set by scanning all of its methods for `this.field = ...` assignments. `static`/`const` fields still use an explicit `static let`/`static const` declaration, since they aren't tied to any particular instance.

`try`/`catch`/`finally` is implemented with `setjmp`/`longjmp`; exceptions are matched by type name string, most-specific catch clause first.

Operator overloading is dynamic: each binary operator call first checks whether the left-hand object's class (or an ancestor) registered a handler, falling back to the builtin behavior otherwise.

## Known limitations of this first pass

- No static type checking beyond class-type tracking described above. Primitive type annotations (`int`, `string`, ...) are accepted but not enforced; a mismatched type surfaces only as a runtime error.
- Generally, we make a best-effort to provide compile errors, but officially, the entirety of invalid Oboe is UB.
- Constructor overload resolution is by argument count only, not by type.
- `import` resolves a module by looking for `<module>.oboe` next to the importing file, or under `.oboe/libraries/<module>.oboe`, and inlines it into the same translation unit with name-prefixing. There's no real package
  repository yet, so `oboe get`/`oboe install` are stubs.
- `oboe build --consolidate` is a no-op: the implementation always produces a single executable (no separate .so/DLL embedding yet).
- `project.json` is read with a minimal targeted scan for the fields this toolchain needs, not a general JSON parser.
