everything will be run through the `oboe` command.

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

i imagine a lot of these will require some flags for various control over things, but this is just a general idea