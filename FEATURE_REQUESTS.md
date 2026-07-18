# Open questions for features to be added

- More build targets than the current three? (`--cc <compiler>` already lets you
  point the build at any toolchain; a new named target is just a default compiler
  + suffix rule.)
- macOS metadata embedding: needs .app bundle generation, which `oboe build`
  doesn't do yet (a note is printed when metadata is given for a macos target).

# Perhaps sometime far in the future

- Add C++ support to `cimport`?
    - Separate syntax or the same?
    - (Trivially already works today if the C++ library exposes `extern "C"` symbols.)