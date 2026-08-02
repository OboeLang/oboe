# The superseded C compiler

**This is not where Oboe is compiled any more.**

It is still built and linked into `bin/oboe` for two reasons:

- It is the reference the selfhost gates diff against. `oboe dump-tokens`, `oboe dump-ast` and `oboe emit-c` run *this* lexer, parser and codegen, and the suite requires `oboec` to produce identical bytes for every file in the tree.
- It is the fallback. `OBOE_USE_C_FRONTEND=1` routes `oboe build` back through it in-process.

Both reasons expire together. The moment `selfhost/` grows a language feature this cannot parse, the gates will fail on a legitimate divergence rather than a
bug, and THIS DIRECTORY SHOULD BE DELETED.

`numfmt.{c,h}` is shared by `codegen.c` and `dump.c`: the one spelling of a float that the whole toolchain agrees on, matching what the runtime's `str()` produces, which is how `selfhost/` reaches the same bytes with a plain `str()`.
