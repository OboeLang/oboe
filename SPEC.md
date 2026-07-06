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
oboe run # Runs a program from main.oboe, or something else as defined by project.json
oboe build # Self-explanatory. Builds the program into an executable, in the dist folder.
           # Embeds all required libraries into various DLLs/.so files, or with the --consolidate / -c
           # flag, all in one executable.
```

- Oboe is a compiled language. The reference implementation is expected to transpile to C and compile with `gcc`.

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

- `let` declares a variable, `const` declares a constant
- Type annotations are optional and go before the variable name

## Primitive types

- `int`s are 32-bit
- `bool`
- `string`s are immutable
- `array`s, which are ordered and may hold more than one type in the same array (not statically homogeneous).
- `dict`

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

## Strings and interpolation

- String literals use double quotes.
- Interpolation is supported inside string literals for embedding
  expressions (exact delimiter syntax is undecided)
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
- Instance methods take an explicit first parameter (`this`) rather than an implicit receiver
- Fields are set via `this.field = value` inside methods.
- Instantiation looks like a function call: `ClassName(args...)`.
- Method calls use dot syntax: `instance.method(args...)`.

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

- Prefer short access paths (e.g. `io.print`) over long chains (are built-ins are namespaced at all?).
    - let's not namespace it
- Whether `print` is a true built-in or lives in a std module (`io`/`std`) is still undecided
    - We COULD make a standard module that is just automatically imported?? and you still call `print()`

## Object model

Oboe leans toward classes/OOP as a first-class construct, though "object-oriented by default" is not quite locked in

---

## Open  questions

- String interpolation syntax: `fstring.oboe` uses `"${name}"` (JS/shell-style) while `classes.oboe` uses `"{this.name}"`
    - use `"${name}"`
- `this` vs. `self` for instance methods
    - this
- Built-in vs. stdlib print
    - built in is more intuitive
- Operator overloading / custom operators
    - yes. both. i like this.
- Domain-specific operators (pipeline `|>`, null-coalescing `??`, spread `...`): raised as a possibility
    - i like having some of these? but i'd need to have like a fuckton of them listed out to me to pick and choose from
- Functional-pattern constructs?
    - ummmmmmm i like the way linq does it how it kinda looks like sql i like that we can do that
- Whether types are always inferred
    - yes unless they're specified
- How OO are we
    - i asked about this in dms