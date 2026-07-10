# Oboe spec (draft)

Many things are still left unresolved; this is incomplete.

## Toolchain

```bash
# Projects and Packages
oboe init # Initializes a project structure.
          # This will be elaborated on.

oboe get <package name> # Installs a library package to the current project, similar to PyPI or NPM.
oboe install <package name> # Installs a program made in Oboe on the repository, like CLI tools.
oboe tidy # Cleans up build/temp files and installs needed packages.

# Running and Building
oboe run helloworld.oboe # Runs a singular Oboe file as-is. Only useful for certain cases, really.
oboe run # Runs a program from main.oboe, or something else as defined by project.json. By default, builds in /tmp (or other OS equivalent), executes, and then deletes the build
oboe build # Self-explanatory. Builds the program into an executable, in the dist folder.
           # Embeds all required libraries into various DLLs/.so files, or with the --consolidate / -c
           # flag, all in one executable.
```

- Oboe is a compiled language. The reference implementation transpiles to C and compiles with `gcc`.

## Syntax

- Brace-delimited blocks (`{ }`). Whitespace is not significant.
- Parentheses around conditions in `if`, `while`, `switch`-like constructs (e.g. `if ( i % 3 == 0 )`, `while (true)`).
- Semicolons are optional.
- Line comments use `//`.
- Logical negation is `!`.
- Logical AND is `&&` *or* the keyword `and`.
- Logical OR is `||` *or* the keyword `or`.
- Modulo is `%`.

## Variables and constants

```
let x = 1           // untyped, type inferred
let int x = 1       // explicitly typed
const x = 1         // untyped constant
const int x = 1 // typed constant
```

- `let` declares a variable, `const` declares a constant.
- Type annotations are optional and go before the variable name.
- Types are inferred unless explicitly specified.

## Primitive types

- `int`s are 32-bit
- `bool`
- `string`s are immutable
- `array`s, which are ordered and may hold more than one type in the same array (not statically homogeneous).
- `dict`
- Primitives have methods.

## Functions

```
int func add(int x, int y) {
    return x + y
}
```

- Declared with the `func` keyword.
- Return type is written before `func`, though it is optional.
- Parameters are `type name` pairs.
- `array args` is the convention for a program's `main` entry point:
  `func main(array args) { ... }`.
- Free functions are allowed; functions do not have to belong to a class.

## Strings and interpolation

- String literals use double quotes.
- Interpolation uses `"${name}"` syntax for embedding expressions.
- String concatenation/formatting can call `str(x)` to convert non-strings.

## Classes

```
class Person {
    func init(this, str name, int age) {
        this.name = name
        this.age = age
    }

    func greet(this) {
        print("Hello! My name is {this.name}, and I'm {str(this.age)} years old!")
    }
}

let john = Person("John", 26)
john.greet()
```

- `class` declares a class; `init` is the constructor method.
- Instance methods take an explicit first parameter (`this`) rather than an implicit receiver.
- Fields are set via `this.field = value` inside methods.
- Instantiation looks like a function call: `ClassName(args...)`.
- Method calls use dot syntax: `instance.method(args...)`.

### Inheritance

- Single inheritance only: a class has at most one parent.
- Method dispatch is resolved at compile time, not via runtime/virtual dispatch.

### Access control

- Members are public by default; `private` is an explicit modifier to restrict access.

### Constructors

- A class may define multiple `init`s (overloading), allowing different construction signatures.
- `static` declares class-level members/functions, shared across all instances and accessed via `ClassName.member` rather than through an instance.
- A class with no `init` at all gets an implicit no-arg, no-op constructor.
- Errors thrown inside `init` can be caught by `try`/`catch` like any other exception.
- Fields can be marked constant/locked so they can't be changed after being set once.

## Control flow

```
for ( i in range(1, n + 1) ) { ... }

if ( cond ) { ... } else if ( cond ) { ... } else { ... }

while ( cond ) { ... }

switch x {
    case 1 { print("one") }
    case 2 { print("two") }
}
```

- `for ... in range(a, b)` iterates a numeric range (upper bound exclusive,
  per `range(1, n + 1)` covering `1..n`).
- `if` / `else if` / `else` with parenthesized conditions.
- `while` with parenthesized condition.
- `switch`/`case` exists, with each `case` given its own `{ }` block body.
- Type checking uses the `is` keyword: `if (100 is int) { ... }`.

## Error handling

```
func main(array args) {
    try {
        ...
    } catch (os.FileNotFoundError e) {
        ...
    } catch (ValueError e) {
        ...
    } catch (Exception e) {
        ...
    } finally {
        ...
    }
}
```

- `try` / `catch` / `finally`, with `catch` supporting multiple, ordered, typed exception clauses (most specific first), each binding a name (`e`) to the caught exception.
- A generic `Exception` type exists as a catch-all.
- File-related exceptions (e.g. `FileNotFoundError`) are expected to live in a file/IO standard-library module (`os.FileNotFoundError`), not the language core.

## Operators

- Operator overloading and custom operators are both supported.
- `??` null-coalescing: `x ?? default`.
- `?.` safe navigation / optional chaining: `user?.address?.city` short-circuits instead of throwing.
- Repetition operator: `x`, e.g. `"ab" x 3` → `"ababab"`.

## Modules / imports

```
import library1
library1.method()

import library3 as l
l.method()
```

- `import <name>` brings in a module, accessed via `<name>.member`.
- `import <name> as <alias>` renames the imported module.
- `import <member> from <name>` imports a specific member.
- `import <member>, <member> from <name>` imports specific members.

## Standard library philosophy

- Prefer short access paths (e.g. `print`) over long chains; built-ins are not namespaced.
- `print` is a built-in, not a stdlib function.

## Project structure

```
my_project/
├── dist            # Not created on init.
├── .gitignore      # Includes dist/ and .oboe/ by default.
├── main.oboe
├── .oboe
│   └── libraries
└── project.json    # See project.example.json for the format.
```

## Object model

Classes are a first-class construct in Oboe.

---

## Open questions

- Inheritance: syntax for declaring a parent, constructor chaining, and override rules.
- Rust-style `?` error-propagation operator: whether it fits Oboe's error-handling model.
- Further domain-specific operators
- Functional-pattern constructs (LINQ-style query syntax is of interest, not yet designed).
- How object-oriented Oboe is by default.
- `repeat` syntax vs `x` syntax (see `unformalizedExamples/things yet to be written.md`)
- Various other things within above file
- Lambda functions or whatever they're called in JS? (the () => {} thing)
- If/Else shorthand?