# Open questions for features to be added

- More build targets than the current three? (`--cc <compiler>` already lets you point the build at any toolchain; a new named target is just a default compiler + suffix rule.)
- macOS metadata embedding: needs .app bundle generation, which `oboe build` doesn't do yet (a note is printed when metadata is given for a macos target).
- In project.json, allow for declaring multiple settings for multiple build targets
    - This allows for running `oboe build` (all defined settings one after the other/the default settings if none are defined) or `oboe build windows` (specifically build the "windows" target as defined in project.json) to quickly build for multiple targets with unique settings for each of them
    - Don't know how necessary this is
- In project.json, separate "meta-xxxx" tags into a "meta" object
- Allow importing from folders, using either the folder's `main.oboe`, or an entry file as specified by a project JSON in the folder
    - Use project name for importing, and folder name if project.json is not present
- Add string iteration `for (c in "test")`
- Add `pairs` and `ipairs` from Lua
    - `for (k, v in pairs(some_iterable))` for key/value pairs
    - `for (i, v in ipairs(any_iterable))` for index/value pairs
- OS library function that returns the root directory of the project/script
- OS library function that returns the current filename of the running script?
- String maniuplation functions
    - `split`
    - `lower`
    - `upper`
    - `reverse`
    - and any others...

# Perhaps sometime far in the future

- Add C++ support to `cimport`?
    - Separate syntax or the same?
    - (Trivially already works today if the C++ library exposes `extern "C"` symbols.)