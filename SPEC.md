# Oboe spec

## Toolchain

```bash
# Projects and Packages
oboe init # Initializes a project structure in the current directory.
    oboe init <directory> # Initializes a project in a new or existing empty directory.

oboe get <package name> # Installs a library package into the current project.
    oboe get <name>@<constraint> # e.g. oboe get http@^1.2.0. Defaults to the latest release.
oboe install <package name> # Installs a program made in Oboe, like CLI tools. Builds it and puts the binary in $OBOE_HOME/bin (default ~/.oboe/bin). Does not need to be run inside a project.
oboe remove <package name> # Un-`get`s a package: deletes its files from .oboe/libraries and drops it from project.jsonc's dependencies.
oboe tidy # Installs whatever project.jsonc declares and .oboe/lock does not already satisfy. Does nothing, and contacts nothing, when everything is present.
          # Should not do anything if not in a project directory.
    -v --verbose # Self-explanatory.
oboe publish # Packs the current project into a kabuk archive and uploads it.
    --dry-run # Print the file list, size and digest without uploading.
oboe sema <file>... # Prints each file's sha256 in the wire form. `-` reads stdin.

# Packages come from a katare registry (see the reedbed repository). The registry is chosen by --registry, else $OBOE_REGISTRY, else "registry" in project.jsonc, else a compiled-in default. Publishing needs a token in $OBOE_TOKEN or in $XDG_CONFIG_HOME/oboe/credentials (mode 0600), one `katare://host:port <token>` per line.
#
# A library is `"kind": "vivlijotiki"` and a tool is `"kind": "pawi"`; `get` and `install` each refuse the other kind rather than installing it somewhere it will never work.
#
# Resolved versions are pinned in .oboe/lock, which is meant to be committed. `oboe init` gitignores the rest of .oboe/ but not that file.

# Running and Building
oboe run helloworld.oboe # Runs a singular Oboe file as-is. Only useful for certain cases, really.
oboe run # Runs a program from main.oboe, or something else as defined by project.jsonc
    oboe run <file> <args>... # Anything after the file is the program's, not oboe's, and
                              # arrives in `main`'s `args` after the program name.
    oboe run -- <args>...     # The same for a project run, where there is no file name in
                              # front of the arguments to separate them from one.
oboe build # Self-explanatory. Builds the program into an executable, in the dist folder.
           # Embeds all required libraries into various DLLs/.so files.
    oboe build <file> # Builds a specific script to an executable of the same name in the current folder.
    -v --verbose # Self-explanatory.
    -o --output # Manually describes the output file. Creates nonexistent folders when specified.
                # e.g. -o my_folder/output.exe will create my_folder.
    -t --target # Names either a build config declared in project.jsonc's `build.targets`,
                # or an OS directly: linux, windows, macos, freebsd, openbsd or netbsd
                # (nt/darwin/osx also accepted). Defaults to the host OS. Windows outputs
                # get .exe appended automatically. windows needs mingw-w64 installed;
                # macos needs osxcross; the BSDs default to clang.
    --emit-c    # Stops after generating the C and writes that instead of an executable,
                # to `-o` if given and otherwise next to where the binary would have gone
                # (`dist/app.c`, or `dist/app.<target>.c` when several targets are built,
                # since they would otherwise overwrite each other). No C compiler is
                # needed or looked for, so `--emit-c -t windows` works without mingw-w64.
    --cc <compiler> # Overrides the C compiler used, for targets/toolchains not covered above.
    --desktop # Generates a desktop-installable artifact for the target: a .desktop
              # launcher on Linux, a .app bundle on macOS. Ignored elsewhere.
    --meta-name / --meta-version / --meta-description / --meta-icon
                # Program metadata. On Windows targets this is embedded as version info
                # (via windres); on macOS it fills in the .app bundle's Info.plist, and on
                # Linux the .desktop file. Defaults come from project.jsonc when building
                # a project.
```

### Build settings in project.jsonc

Build settings may also live under a `"build"` object, with CLI flags always taking
precedence. Program metadata goes in a nested `"meta"` object (the older flat
`"meta-name"` / `"meta-version"` / ... keys are still read as a fallback).

`"build.targets"` declares named build configurations. Each may set any build field;
one with no `"target"` of its own targets the OS it is named after.

```json
"build": {
    "output": "dist/app",
    "meta": { "name": "My App", "version": "1.0.0", "icon": "myicon.ico" },
    "targets": {
        "linux":   { "output": "dist/app-linux", "desktop": true },
        "windows": { "output": "dist/app.exe" },
        "portable": { "target": "linux", "output": "dist/app-portable" }
    }
}
```

- `oboe build` with `targets` declared builds every one of them in turn, each with its
  own settings; without it, it builds once from the plain `build` settings.
- `oboe build -t windows` builds just that config. A `-t` value that isn't a declared
  target name is treated as an OS name, as before.
- `oboe build <file>` always means a single script, never a target name.

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
var x = 1        // untyped, type inferred
int x = 1        // explicitly typed
const var x = 1  // untyped constant
const int x = 1  // typed constant
```

- `var` declares an untyped variable, `const` declares a constant.
- Type annotations are optional and replace `var`.
- Types are inferred unless explicitly specified.

### Scoping

- Variables are block-scoped: a variable declared inside `{ }` ends at the closing brace and does not leak into the enclosing block.
- Each `for` iteration gets a fresh binding of the loop variable.
- A variable declared at the top level of a file is scoped to that file (module). Other files reach it through the module: `mymodule.counter`, or `import counter from mymodule`. There is no separate `global` keyword, module-level state is the global story.
- Top-level statements in an imported module run once at program startup, before `main` (modules first, then the main file's own top level).

## Primitive types

- `int` is the default integer type: 64-bit signed, and it does not wrap on its own.
- Sized integers: `int8`, `int16`, `int32`, `int64` and the unsigned `uint8`, `uint16`,
  `uint32`, `uint64` (`uint` is `uint64`).
- `float` is 64-bit; `float64` is a synonym for it, and `float32` differs only in
  rounding its stores through single precision.
- `bool`
- `string`s are immutable
- `array`s, which are ordered and may hold more than one type in the same array (not statically homogeneous).
- `dict`
- Primitives have methods.

### Numeric semantics

Type annotations are enforced at stores. Assigning into a variable, parameter or field declared with a sized integer type wraps the value into that type's range; a `float32` store rounds through single precision. Plain `int` and `float` never truncate.

```
int8 a = 127
a = a + 1     // -128, wrapped
uint8 b = 200
int16 c = a + b   // arithmetic promotes to the wider operand
```

Arithmetic promotes to the wider operand, and to unsigned when the widest operand is unsigned (C's usual arithmetic conversions). Two plain `int`s stay a plain `int`.

Division between two integers is integer division; any float operand makes it real division, where dividing by zero yields infinity or NaN rather than erroring.

```
7 / 2      // 3
7.0 / 2    // 3.5
```

`is` with a sized type asks whether the value *fits* that type's range, not how it was declared: `127 is int8` is true and `200 is int8` is false.

## Functions

```
int func add(int x, int y) {
    return x + y
}
```

- Declared with the `func` keyword.
- Return type is written before `func`, though it is optional.
- Parameters are `type name` pairs.
- `array args` is the convention for a program's `main` entry point: `func main(array args) { ... }`.
- Free functions are allowed; functions do not have to belong to a class.

### Optional parameters and named arguments

A parameter may carry a default, making it optional. Defaulted parameters must come last, so a positional call is never ambiguous.

```
func hello(str name = "Jade") {
    print("Hello, ${name}!")
}

hello()          // Hello, Jade!
hello("Robin")   // Hello, Robin!
```

Arguments can also be given by name, mixed with positional ones. Positional arguments fill parameters in order and must all come before the named ones.

```
func code(int one = 1, int two = 2, int three = 3, int four = 4) { ... }

code(6, three = 5)   // one by position, three by name; two and four keep their defaults
```

A default is an expression, evaluated at each call site that omits it. That means it can refer to module-level state but not to the function's own earlier parameters (`func f(int a = 1, int b = a)` is an error).

Omitting a parameter that has no default is a compile error naming it (`missing required argument 'x' in call to 'f'`), as are an unknown parameter name, an argument given twice, and too many arguments. This applies to constructors and methods as well as free functions; a constructor overload is still selected by argument count alone, never by type.

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

var john = Person("John", 26)
john.greet()
```

- `class` declares a class; `init` is the constructor method.
- Instance methods take an explicit first parameter (`this`) rather than an implicit receiver.
- Fields are set via `this.field = value` inside methods.
- Instantiation looks like a function call: `ClassName(args...)`.
- Method calls use dot syntax: `instance.method(args...)`.

### Inheritance

```
class Dog extends Animal {
    func init(this, string name) {
        super(name)      // chains to Animal's constructor
        this.tricks = 0
    }

    func speak(this) {   // overrides Animal.speak by shadowing it
        super.speak()    // the parent's version is still reachable
        print("woof")
    }
}
```

- Single inheritance only: a class has at most one parent, declared with `extends`.
- Method dispatch is resolved at compile time, not via runtime/virtual dispatch.
- A child method with the same name as a parent method shadows it; there is no `override` keyword. `super.method()` calls the parent's version.
- `super(args...)` inside `init` chains to the nearest ancestor constructor.
- A class that declares no `init` of its own inherits its ancestor's constructors (`Cat("Whiskers")` works if `Animal` has a one-arg `init`).


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

- `for ... in range(a, b)` iterates a numeric range (upper bound exclusive, per `range(1, n + 1)` covering `1..n`).
- `for (x in iterable)` walks an array's elements, a dict's values, or a string one character at a time: `for (c in "test")` binds `c` to `"t"`, `"e"`, `"s"`, `"t"`.
- `for (k, v in pairs(x))` binds key/value pairs: the keys of a dict, the indices of an array or string. `for (i, v in ipairs(x))` binds index/value pairs for any iterable. Both need exactly two loop variables, and two loop variables need one of them.
- `if` / `else if` / `else` with parenthesized conditions.
- `while` with parenthesized condition.
- `break` leaves the innermost enclosing loop and `continue` skips to its next iteration. Both work in every loop form. A `case` body is not a loop — `switch` has no fallthrough, so there is nothing there to break out of, and a `break` inside one leaves the loop around the `switch` (outside any loop it is a compile error).
- `switch`/`case` exists, with each `case` given its own `{ }` block body.
- Type checking uses the `is` keyword: `if (100 is int) { ... }`.
- Ternary/if-else shorthand: `cond ? a : b` (right-associative, binds looser than `??`): `var label = x % 2 == 0 ? "even" : "odd"`.

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
- Bitwise operators on integers: `&`, `|`, `^`, `~`, `<<`, `>>`. They follow C's precedence -- shifts bind between the additive and comparison operators, and `&`, `^`, `|` bind between equality and any user-declared operators, tightest first. The result takes the promoted width of its operands; shifting an unsigned value shifts in zeros. A float operand is an error. All six can be overloaded on a class like any other operator.

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
- Members include a module's top-level variables, not just its functions.
- OS-specific module files: when compiling for a given target OS, a file named `foo.<os>.oboe` (e.g. `foo.windows.oboe`) is preferred over `foo.oboe` for `import foo`. Useful for per-OS `cimport`s with a shared generic fallback. OS names match the build targets: `linux`, `windows`, `macos`, `freebsd`, `openbsd`, `netbsd`.
- A module may be a folder rather than a single file. A folder containing a project.jsonc is imported under that project's `name`, entering through its `entry` file; a folder without one is imported under its own directory name, entering through its `main.oboe`. A folder module resolves its own imports relative to itself, so a library can import its siblings by bare name.
- Resolution order for `import foo`, first in the importing file's own directory and then in `.oboe/libraries`: `foo.<target-os>.oboe`, `foo.oboe`, then a module folder.

## Standard library philosophy

- Prefer short access paths (e.g. `print`) over long chains; built-ins are not namespaced.
- `print` is a built-in, not a stdlib function.

- `print` and `write` take any number of arguments: none prints an empty line, several are joined with spaces (like Python): `print("x =", x)`.
- `write()` - Print without newline.
- `eprint()` / `ewrite()` - The same pair, writing to stderr instead. stdout is flushed first, so the two streams stay in order when both go to the same place.
- `input()` - Pauses execution and waits for user input, returns that input. Same as Python.
- `ord(s)` - The numeric value of a string's first byte; `chr(n)` is the inverse. Byte-oriented like the rest of the string handling, so `ord` on a multi-byte character gives its first byte. `ord("")` throws `ValueError`, and `chr` throws `ValueError` outside 0..255 — including `chr(0)`, which a NUL-terminated string cannot hold.

### Built-in stdlib modules

Importing `math`, `random` or `os` works with no file on disk, they're built into the language runtime. (A file of the same name next to your code still wins, so nothing is reserved.)

- `math.abs(n)`, `math.min(a, b)`, `math.max(a, b)`, `math.pow(base, exp)`, `math.sqrt(n)`, `math.floor(n)`, `math.ceil(n)`, `math.round(n)`. Integer arguments get exact integer math (`pow` is integer exponentiation, `sqrt` is the floor square root); as soon as any argument is a float the result is floating point. `floor`/`ceil`/`round` always return an int.
- `random.seed(n)`, `random.randint(lo, hi)` (inclusive on both ends, like Python), `random.choice(array)` (a deterministic PRNG: the same seed gives the same sequence on every platform.)
- `os.run(cmd)` runs a command through the shell and returns its exit code; `os.spawn(cmd)` starts it without waiting and returns the pid.
- `os.read_file(path)` (throws `os.FileNotFoundError`), `os.write_file(path, content)`, `os.append_file(path, content)`, `os.exists(path)`, `os.remove(path)`, `os.getenv(name)` (string, or `null` when unset), `os.exit(status)` (ends the program immediately, skipping any `finally`).
- `os.is_dir(path)`, `os.mkdir(path)` (recursive, like `mkdir -p`; an existing directory counts as success), `os.listdir(path)` (entry names without `.` and `..`, sorted by byte order so a directory walk is reproducible; throws `os.FileNotFoundError` when the path isn't a readable directory). `os.exists` cannot tell a directory from a file — that is what `os.is_dir` is for.
- `os.realpath(path)` is the canonical absolute path, with `.`, `..` and symlinks resolved, or `null` when the path does not resolve. Two names for the same file always give the same string.
- `os.script_file()` is the absolute path of the running script and `os.script_dir()` its directory; `os.project_root()` is the nearest ancestor directory containing a project.jsonc, falling back to `os.script_dir()`. All three are resolved at compile time.

### Methods on primitives

Strings, arrays and dicts carry methods. Because the compiler does not track primitive types, the receiver's type is checked when the method runs: calling one on the wrong kind of value throws a catchable `TypeError`.

- Any: `.str()`
- Shared: `.len()`, `.contains(x)`, `.index_of(x)`, `.reverse()`, `.slice(start, end)`
- Strings: `.upper()`, `.lower()`, `.trim()`, `.split(sep)`, `.starts_with(s)`, `.ends_with(s)`, `.replace(from, to)`, `.substr(start, len)`, `.repeat(n)`, `.to_int()`, `.to_float()` (the conversions throw `ValueError` on unparsable input)
- Arrays: `.push(v)`, `.pop()`, `.insert(i, v)`, `.remove_at(i)`, `.join(sep)`
- Dicts: `.keys()`, `.values()`, `.has(k)`, `.remove(k)`

`.push`/`.pop`/`.insert`/`.remove_at` mutate the array in place; `.reverse()` and `.slice()` return new values. `.split("")` splits into single characters, and `.index_of` returns -1 when absent.

```
"a,b,c".split(",")   // ["a", "b", "c"]
"Hello".upper()      // "HELLO"
[1, 2, 3].join("-")  // "1-2-3"
```

## Project structure

```
my_project/
├── dist            # Not created on init.
├── .gitignore      # Includes dist/ and .oboe/ by default.
├── main.oboe
├── .oboe
│   └── libraries
└── project.jsonc   # See project.example.jsonc for the format.
```

## Object model

Classes are a first-class construct in Oboe.

> [!NOTE]
> Perhaps add examples?

# Events System

The events system is a powerful system, akin to the broadcast system from Scratch.

This is built into the language.

`on <event> { ... }` - runs code when an event happens
```
on KeyboardInterruptEvent {
    print("Quitting...")
}
```

`on <event> as <variable> { ... }` - runs code when an event happens, passes data to the variable.
```
on ExampleEvent as e {
    print(e.name)
}
```

`event` - type, used to create events. could also be `Event`, but i dunno
```
event MyEvent = event()

func main(array args) {
    MyEvent.fire()
}

on MyEvent {
    print("woah")
}
```

events may also come with data.

```
event MyEvent = event(str name)

MyEvent.fire("Jade")
MyEvent.fire("Robin")

on MyEvent as e {
    print(e.name) // prints "Jade" the first time and "Robin" the second time
}
```

`event.fire()` - fires/broadcasts the event, appropriate data goes in.

maybe other methods and such.

## Built-in events

`KeyboardInterruptEvent` - gets sent whenever the user inputs ^C (Ctrl+C) in the terminal
```
on KeyboardInterruptEvent {
    print("Quitting...")
}
```
important for the compiler: here's what should happen when a keyboard interrupt happens:

- keyboard interrupt
- stop everything
- fire that event
- when that finishes, THEN quit

BUT, if another interrupt is sent while that code is running, then it just quits immediately

# Custom operators

Oboe allows users to declare custom operators.

Use the `operator <operator> (type a, type b) { ... }` syntax. Then, return an object.
```
operator ||> (int a, int b) {
    var c = a * 10
    return c + b
}

print(5 ||> 8) // prints 58
```

When defining an operator, you are given two variables. 

`a`, which is the left side, and `b`, the right side.

# Operator overloading

```
class Vector2 {
    func init(this, float x, float y) {
        let this.x = x
        let this.y = y
    }

    operator + (this, Vector2 other) {
        return Vector2(this.x + other.x, this.y + other.y)
    }
}
```
This example overrides the + operation, specifically between a Vector2 and another Vector2.

# FFI

C functions are imported from shared libraries with `cimport`:

```
cimport strlen from "libc.so.6"
cimport abs from "libc.so.6"

func main(array args) {
    print(strlen("hello")) // prints 5
    print(abs(-42))        // prints 42
}
```

`cimport <symbol> from "<library>"` resolves `<symbol>` in `<library>` at program startup and makes it callable like a normal function. The string operand distinguishes it from a module member import (`import member from module`).

Arguments and return values are word-sized: ints, bools, and nulls pass by value, strings pass as C string pointers, and the return value comes back as an int. Calls take at most 8 arguments. Floats, structs, and out-parameters are not yet supported.


## Open questions

- How object-oriented Oboe is by default.
