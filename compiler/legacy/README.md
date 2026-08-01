# The superseded C compiler

**This is not where Oboe is compiled any more.** The compiler proper lives in
`../selfhost/`, is written in Oboe, and is built into `bin/oboec`, which
`bin/oboe` execs for every `run`, `build` and `install`. Nothing here is on
that path. Don't add features to it, and don't reach for it as an example of
how the toolchain works today.

It is still built and linked into `bin/oboe` for two reasons, both of which are
about checking the Oboe compiler rather than about compiling anything:

- **It is the reference the selfhost gates diff against.** `oboe dump-tokens`,
  `oboe dump-ast` and `oboe emit-c` run *this* lexer, parser and codegen, and
  the suite requires `oboec` to produce identical bytes for every file in the
  tree. That comparison is the only thing standing between a bug in
  `selfhost/codegen.oboe` and a compiler that quietly miscompiles. `dump.{c,h}`
  exists solely for those gates, and defines the format its twin
  `selfhost/dump.oboe` has to reproduce.
- **It is the fallback.** `OBOE_USE_C_FRONTEND=1` routes `oboe build` back
  through it in-process. That is deprecated and undocumented outside this file;
  it exists so a bad day for `oboec` is recoverable without a git checkout. CI
  runs the suite through it once so it cannot rot into a fallback that doesn't
  work.

Both reasons expire together. The moment `selfhost/` grows a language feature
this cannot parse, the gates will fail on a legitimate divergence rather than a
bug, and that is the signal to delete this directory -- accepting that what
remains is the golden suite plus `make bootstrap-check`, and that a fixpoint
proves the compiler is consistent with itself, not that it is correct.

`numfmt.{c,h}` is shared by `codegen.c` and `dump.c`: the one spelling of a
float that the whole toolchain agrees on, matching what the runtime's `str()`
produces, which is how `selfhost/` reaches the same bytes with a plain `str()`.
