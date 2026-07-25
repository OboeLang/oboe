# Open questions for features to be added

- Floats: `float`/`float64` (64-bit) and `float32` exist. Anything else worth adding?
    - A `decimal` fixed-point type, for exact base-10 arithmetic?
- Strings are byte-oriented everywhere (`for (c in s)`, `.len()`, `.reverse()`), so
  multi-byte UTF-8 characters get split. Worth making them codepoint-aware?

# Perhaps sometime far in the future

- Add C++ support to `cimport`?
    - Separate syntax or the same?
    - (Trivially already works today if the C++ library exposes `extern "C"` symbols.)
