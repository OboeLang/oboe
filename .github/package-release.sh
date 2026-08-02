#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-only
# © 2026 Sushii64
# © 2026 robinpie
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# Packs a built tree into the release tarball. Run from compiler/:
#
#     ../.github/package-release.sh <platform> <version>
#
# The layout is not cosmetic. compile_c_to_binary() in src/main.c builds every
# Oboe program against "<dir containing oboe>/../runtime/oboe_runtime.c", found
# on disk at build time rather than embedded, so the runtime sources ship
# alongside the binaries and must keep exactly that relative position. A
# tarball of the two binaries alone would install cleanly and then fail on the
# user's first `oboe run`.
set -euo pipefail

platform="${1:?usage: package-release.sh <platform> <version>}"
version="${2:?usage: package-release.sh <platform> <version>}"

name="oboe-$version-$platform"
out="dist-release"
stage="$out/$name"

rm -rf "$out"
mkdir -p "$stage/bin" "$stage/runtime"

cp bin/oboe bin/oboec "$stage/bin/"
# oboe_runtime.c is compiled on the user's machine, so it is a shipped source
# file rather than a build leftover
cp runtime/oboe_runtime.c runtime/oboe_runtime.h "$stage/runtime/"
cp ../LICENSE.txt "$stage/"
cp ../SPEC.md "$stage/"

# -g is in CFLAGS for development; debug info roughly triples the tarball and
# is of no use to someone installing a release
strip "$stage/bin/oboe" "$stage/bin/oboec" 2>/dev/null || true

cat > "$stage/README.md" <<EOF
# Oboe $version ($platform)

Oboe compiles to C and then invokes a C compiler, so **this package is not a
self-contained toolchain**: you also need \`gcc\` (or \`clang\`, via
\`--cc\`) on your PATH. On macOS that means the Command Line Tools
(\`xcode-select --install\`).

## Install

Keep \`bin/\` and \`runtime/\` together -- \`oboe\` locates the runtime sources
relative to its own executable, and moving the binary out on its own will break
every build. Put the whole directory somewhere permanent and link the binary:

    sudo mv $name /usr/local/lib/oboe
    sudo ln -s /usr/local/lib/oboe/bin/oboe /usr/local/bin/oboe

A symlink is fine; \`oboe\` resolves it before looking for the runtime.

    oboe --version
    oboe init hello && cd hello && oboe run

## What is in here

    bin/oboe        the CLI and package manager
    bin/oboec       the compiler proper, itself written in Oboe
    runtime/        compiled into each program you build
    SPEC.md         the language reference

## Supported platforms

Linux (x86_64) and macOS (Apple Silicon). There is no Intel Mac build: the
only free x86_64 macOS CI runner has been retired

There is no Windows build either: the CLI is POSIX-throughout
(\`fork\`/\`execv\`/\`waitpid\`, \`mkstemps\`, \`dirname\`), so use WSL there.
\`oboe build -t windows\` cross-compiles Windows *programs* from a supported
host, but that path has no test coverage yet.
EOF

tar -czf "$out/$name.tar.gz" -C "$out" "$name"
rm -rf "$stage"
echo "packaged $out/$name.tar.gz"
ls -lh "$out"
