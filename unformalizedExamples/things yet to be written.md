# Clarification on Oboe CLI

```bash
# Projects and Packages
oboe init # Initializes a project structure in the current directory.
    oboe init <directory> # Initializes a project in a new or existing empty directory.

oboe get <package name> # Installs a library package to the current project, similar to PyPI or NPM.
oboe install <package name> # Installs a program made in Oboe on the repository, like CLI tools.
oboe tidy # Cleans up build/temp files and installs needed packages.
          # Should not do anything if not in a project directory.
    -v --verbose # Self-explanatory.

# Running and Building
oboe run helloworld.oboe # Runs a singular Oboe file as-is. Only useful for certain cases, really.
oboe run # Runs a program from main.oboe, or something else as defined by project.json
oboe build # Self-explanatory. Builds the program into an executable, in the dist folder.
           # Embeds all required libraries into various DLLs/.so files.
    -v --verbose # Self-explanatory.
    -o --output # Manually describes the output file. Creates nonexistent folders when specified.
                # e.g. -o my_folder/output.exe will create my_folder.
```

Any other flags and functionalities can be added to this list as needed.

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
let event MyEvent = event()

func main(array args) {
    MyEvent.fire()
}

on MyEvent {
    print("woah")
}
```

events may also come with data.

```
let event MyEvent = event(str name)

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

# Other

`repeat <n> { ... }` - Self explanatory.

```
repeat 3 {
    print("hello!")
}

// hello!
// hello!
// hello!
```

OR!!!! I haven't decided yet: Allowing `x` to be used on function calls

```
print("hello!") x 3

// hello!
// hello!
// hello!
```