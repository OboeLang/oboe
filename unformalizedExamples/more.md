i lovethis messy folder

# FFI

we can go about this a few ways

the ideal method:

here is our single add function for demonstration:
```c
int add(int x, int y) {
    return x + y;
}
```
this is built to mathlib.so

```
import add from mathlib // normal, oboe library
cimport add from "mathlib.so" // import from c/cpp

// with this method you'd also be able to do this

cimport "mathlib.so" as math // you'd HAVE to do `as` when doing this

math.add(2, 5)
```

if that won't work due to needing to define types and arguments theennnn something like this?

```
cimport add {
    int x
    int y
    return int
} from "mathlib.so"
```
or something i don't really know how these things work

# Other shit idk

`input()` - pauses execution and waits for user input, returns that input. same as python
`write()` - print without newline

# Changing the way Variables are formatted

this is it now
```
var x = 1 // untyped
int x = 1 // typed
const var x = 1 // constant untyped
const int x = 1 // constant typed
```