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
# Runs every tests/*.oboe file and diffs its output against tests/*.expected.
# A test that is expected to fail compilation has a .expect_fail file instead,
# whose contents (if non-empty) must appear in the compiler's stderr.
cd "$(dirname "$0")/.." || exit 1

OBOE=bin/oboe
pass=0
fail=0
# Some tests need a C compiler for a helper, or a network stack. They are
# skipped rather than failed where that is genuinely absent, and the count is
# reported so a silently-shrinking suite is visible.
skip=0

# Mirrors HOST_OS in src/main.c, so a per-OS fixture is suffixed the same way an
# OS-specific module is (foo.macos.expected alongside foo.macos.oboe).
# Overridable so the selection itself can be exercised from one machine; note
# that only the fixture *choice* changes, the compiler still targets this host.
if [ -z "$HOST_OS" ]; then
    case "$(uname -s)" in
        Darwin) HOST_OS=macos ;;
        Linux)  HOST_OS=linux ;;
        CYGWIN*|MINGW*|MSYS*) HOST_OS=windows ;;
        *)      HOST_OS="$(uname -s | tr '[:upper:]' '[:lower:]')" ;;
    esac
fi

# Some output is legitimately platform-specific. Prefer <name>.<os>.<kind>, fall
# back to the shared <name>.<kind>; prints nothing if neither exists.
fixture() {
    if [ -f "$1.$HOST_OS.$2" ]; then
        printf '%s' "$1.$HOST_OS.$2"
    elif [ -f "$1.$2" ]; then
        printf '%s' "$1.$2"
    fi
}

for src in tests/*.oboe; do
    name="${src%.oboe}"
    base="$(basename "$name")"
    # helper modules (imported by other tests) start with _ and aren't run directly
    case "$base" in _*) continue ;; esac

    expect_fail="$(fixture "$name" expect_fail)"
    if [ -n "$expect_fail" ]; then
        out="$($OBOE run "$src" 2>&1)"
        rc=$?
        want="$(cat "$expect_fail")"
        if [ $rc -ne 0 ] && { [ -z "$want" ] || printf '%s' "$out" | grep -qF "$want"; }; then
            echo "PASS $base (failed as expected)"
            pass=$((pass+1))
        else
            echo "FAIL $base — expected compile/run failure containing: $want"
            printf '%s\n' "$out" | sed 's/^/    /'
            fail=$((fail+1))
        fi
        continue
    fi

    expected="$(fixture "$name" expected)"
    if [ -z "$expected" ]; then
        echo "SKIP $base (no .expected file)"
        continue
    fi
    # a test reading from input() provides its stdin in a .stdin file
    if [ -f "$name.stdin" ]; then
        out="$($OBOE run "$src" <"$name.stdin" 2>&1)"
    else
        out="$($OBOE run "$src" </dev/null 2>&1)"
    fi
    rc=$?
    if [ $rc -eq 0 ] && [ "$out" = "$(cat "$expected")" ]; then
        echo "PASS $base"
        pass=$((pass+1))
    else
        echo "FAIL $base (exit $rc)"
        diff <(printf '%s\n' "$out") "$expected" | sed 's/^/    /'
        fail=$((fail+1))
    fi
done

# --- CLI behavior tests ---

# a failing `oboe build` must not leave a dist/ directory behind
tmp="$(mktemp -d)"
cat > "$tmp/main.oboe" <<'EOF'
func main(array args) {
    print(no_such_variable)
}
EOF
printf '{\n    "project": { "name": "broken", "entry": "main.oboe" }\n}\n' > "$tmp/project.json"
( cd "$tmp" && "$OLDPWD/$OBOE" build ) >/dev/null 2>&1
rc=$?
if [ $rc -ne 0 ] && [ ! -d "$tmp/dist" ]; then
    echo "PASS build_no_dist_on_error"
    pass=$((pass+1))
else
    echo "FAIL build_no_dist_on_error (exit $rc, dist exists: $([ -d "$tmp/dist" ] && echo yes || echo no))"
    fail=$((fail+1))
fi
rm -rf "$tmp"

# `oboe remove` deletes the package's files and its dependency entry, leaving
# the rest of project.json untouched
tmp="$(mktemp -d)"
mkdir -p "$tmp/.oboe/libraries/mylib"
touch "$tmp/.oboe/libraries/mylib.oboe" "$tmp/.oboe/libraries/mylib/main.oboe"
cat > "$tmp/project.json" <<'EOF'
{
    "project": { "name": "p", "entry": "main.oboe" },
    "dependencies": {
        "oboe": ">=1.0.0",
        "mylib": ">=1.0.0"
    }
}
EOF
( cd "$tmp" && "$OLDPWD/$OBOE" remove mylib ) >/dev/null 2>&1
if [ ! -e "$tmp/.oboe/libraries/mylib.oboe" ] && [ ! -d "$tmp/.oboe/libraries/mylib" ] &&
   ! grep -q mylib "$tmp/project.json" && grep -q '"oboe"' "$tmp/project.json" &&
   ! tr -d '[:space:]' < "$tmp/project.json" | grep -q ',}'; then
    echo "PASS remove_package"
    pass=$((pass+1))
else
    echo "FAIL remove_package"
    cat "$tmp/project.json" | sed 's/^/    /'
    fail=$((fail+1))
fi
rm -rf "$tmp"

# `oboe build` with a build.targets object builds every declared target
tmp="$(mktemp -d)"
cat > "$tmp/main.oboe" <<'EOF'
func main(array args) { print("hi") }
EOF
cat > "$tmp/project.json" <<'EOF'
{
    "project": { "name": "multi", "entry": "main.oboe" },
    "build": {
        "targets": {
            "one": { "target": "linux", "output": "dist/one" },
            "two": { "target": "linux", "output": "dist/two" }
        }
    }
}
EOF
( cd "$tmp" && "$OLDPWD/$OBOE" build ) >/dev/null 2>&1
if [ -x "$tmp/dist/one" ] && [ -x "$tmp/dist/two" ]; then
    echo "PASS build_all_targets"
    pass=$((pass+1))
else
    echo "FAIL build_all_targets (dist: $(ls "$tmp/dist" 2>/dev/null | tr '\n' ' '))"
    fail=$((fail+1))
fi
rm -rf "$tmp"

# ---- self-hosting spike -------------------------------------------------
#
# selfhost/mini/ is a compiler for a small Oboe-like subset, written in Oboe.
# It is the standing proof that the language can host its own toolchain, and it
# exercises end to end the pieces the real self-hosted compiler depends on:
# break/continue in scanning loops, short-circuit and/or, ord() classification,
# dict-shaped AST nodes, C string escaping and eprint+os.exit diagnostics.
tmp="$(mktemp -d)"
if "$OBOE" build selfhost/mini/main.oboe -o "$tmp/mini" >/dev/null 2>&1 &&
   "$tmp/mini" selfhost/mini/sample.mini -o "$tmp/sample" >/dev/null 2>&1; then
    got="$("$tmp/sample" 2>&1)"
    if [ "$got" = "$(cat selfhost/mini/sample.expected)" ]; then
        echo "PASS selfhost_mini"
        pass=$((pass+1))
    else
        echo "FAIL selfhost_mini (output mismatch)"
        diff <(printf '%s\n' "$got") selfhost/mini/sample.expected | sed 's/^/    /'
        fail=$((fail+1))
    fi
else
    echo "FAIL selfhost_mini (could not build the spike or compile the sample)"
    fail=$((fail+1))
fi

# a diagnostic goes to stderr with a non-zero status and no exception wrapping,
# which is the whole point of eprint + os.exit
printf 'func main() {\n    var x = 1 +\n}\n' > "$tmp/bad.mini"
err="$("$tmp/mini" "$tmp/bad.mini" --emit-c 2>&1 >/dev/null)"
rc=$?
if [ $rc -ne 0 ] && printf '%s' "$err" | grep -q "error: expected an expression"; then
    echo "PASS selfhost_mini_diagnostic"
    pass=$((pass+1))
else
    echo "FAIL selfhost_mini_diagnostic (exit $rc): $err"
    fail=$((fail+1))
fi
rm -rf "$tmp"

# ---- self-hosted lexer --------------------------------------------------
#
# selfhost/lexer.oboe is the port of src/lexer.c, and this is the gate on it:
# `oboe dump-tokens` and `oboec --dump-tokens` must agree byte for byte -- token
# type, line and escaped lexeme -- over every Oboe file in the tree, plus the
# torture fixtures for the corners the real tests miss and a deliberately bad
# character for the diagnostic. Exit status is compared too, so an error that
# prints the right text with the wrong status still fails.
tmp="$(mktemp -d)"
if "$OBOE" build selfhost/main.oboe -o "$tmp/oboec" >/dev/null 2>&1; then
    printf 'var a = 1\nvar b = `x`\n' > "$tmp/badchar.oboe"
    diffs=""
    n=0
    : > "$tmp/seen"
    for src in tests/*.oboe tests/helpers/*.oboe selfhost/*.oboe \
               selfhost/mini/*.oboe "$tmp/badchar.oboe"; do
        n=$((n+1))
        a="$("$OBOE" dump-tokens "$src" 2>&1; printf 'rc=%s' "$?")"
        b="$("$tmp/oboec" --dump-tokens "$src" 2>&1; printf 'rc=%s' "$?")"
        [ "$a" = "$b" ] || diffs="$diffs $src"
        printf '%s\n' "$a" | awk '{print $1}' >> "$tmp/seen"
    done
    if [ -z "$diffs" ]; then
        echo "PASS selfhost_lexer ($n files)"
        pass=$((pass+1))
    else
        echo "FAIL selfhost_lexer (differs:$diffs)"
        fail=$((fail+1))
    fi

    # Agreeing on tokens the corpus never produces proves nothing, so pin the
    # coverage: every TokenType in the enum has to show up somewhere above.
    # T_X_OP is the one exception -- the lexer never emits it, the parser
    # retags an identifier spelled `x` in operator position.
    sed -n '/^typedef enum {/,/^} TokenType;/p' src/lexer.h |
        grep -o '\bT_[A-Z_0-9]*' | grep -v '^T_X_OP$' | sort -u > "$tmp/want"
    sort -u "$tmp/seen" > "$tmp/got"
    missing="$(comm -23 "$tmp/want" "$tmp/got" | tr '\n' ' ')"
    if [ -z "$missing" ]; then
        echo "PASS selfhost_lexer_coverage ($(wc -l < "$tmp/want" | tr -d ' ') token types)"
        pass=$((pass+1))
    else
        echo "FAIL selfhost_lexer_coverage (no corpus file produces: $missing)"
        fail=$((fail+1))
    fi

    # ---- self-hosted parser ---------------------------------------------
    #
    # The same gate one stage further on: `oboe dump-ast` against
    # `oboec --dump-ast`, over the same corpus plus every malformed input in
    # tests/helpers/parse_errors.txt. The AST dump carries each node's kind,
    # line and every field of its arm in src/ast.h, so this catches a
    # mis-shaped tree and not just one that happens to print the same.
    mkdir -p "$tmp/pe"
    while IFS="$(printf '\t')" read -r name src; do
        case "$name" in ''|'#'*) continue ;; esac
        printf '%s\n' "$src" > "$tmp/pe/$name.oboe"
    done < tests/helpers/parse_errors.txt

    diffs=""
    n=0
    : > "$tmp/kinds"
    for src in tests/*.oboe tests/helpers/*.oboe selfhost/*.oboe \
               selfhost/mini/*.oboe "$tmp/pe"/*.oboe; do
        n=$((n+1))
        a="$("$OBOE" dump-ast "$src" 2>&1; printf 'rc=%s' "$?")"
        b="$("$tmp/oboec" --dump-ast "$src" 2>&1; printf 'rc=%s' "$?")"
        [ "$a" = "$b" ] || diffs="$diffs $src"
        printf '%s\n' "$a" | grep -oE '\b(EXPR|STMT|DECL|FOR)_[A-Z_]+' \
            >> "$tmp/kinds" || true
    done
    if [ -z "$diffs" ]; then
        echo "PASS selfhost_parser ($n files)"
        pass=$((pass+1))
    else
        echo "FAIL selfhost_parser (differs:$diffs)"
        fail=$((fail+1))
    fi

    # As with the tokens: agreement over kinds the corpus never builds proves
    # nothing, so every ExprKind, StmtKind, DeclKind and ForIterKind in ast.h
    # has to appear in one of the dumps above.
    { sed -n '/^typedef enum {/,/} ExprKind;/p;/^typedef enum {/,/} StmtKind;/p
              /^typedef enum {/,/} DeclKind;/p' src/ast.h
      grep 'ForIterKind' src/ast.h
    } | grep -oE '\b(EXPR|STMT|DECL|FOR)_[A-Z_]+' | sort -u > "$tmp/kwant"
    sort -u "$tmp/kinds" > "$tmp/kgot"
    missing="$(comm -23 "$tmp/kwant" "$tmp/kgot" | tr '\n' ' ')"
    if [ -z "$missing" ]; then
        echo "PASS selfhost_parser_coverage ($(wc -l < "$tmp/kwant" | tr -d ' ') AST kinds)"
        pass=$((pass+1))
    else
        echo "FAIL selfhost_parser_coverage (no corpus file produces: $missing)"
        fail=$((fail+1))
    fi
else
    echo "FAIL selfhost_lexer (could not build selfhost/main.oboe)"
    fail=$((fail+1))
fi
rm -rf "$tmp"

# ---- katare client ------------------------------------------------------
#
# Driven against tests/helpers/katare_stub.c, a scripted server compiled here.
# The real registry lives in the reedbed repository, which this repo's CI does
# not check out -- and the stub can be made to misbehave in ways a correct
# server never would, which is most of what is worth testing on the client.

stub_bin=""
stub_dir="$(mktemp -d)"
if ${CC:-gcc} -std=c11 -D_POSIX_C_SOURCE=200809L -o "$stub_dir/stub" \
        tests/helpers/katare_stub.c 2>/dev/null; then
    stub_bin="$stub_dir/stub"
else
    echo "SKIP katare_client (no C compiler for the test helper)"
    skip=$((skip+1))
fi

# a stub that outlives the suite would hold its port and hang the next run
stub_pid=""
kill_stub() { [ -n "$stub_pid" ] && kill "$stub_pid" 2>/dev/null; stub_pid=""; }
trap 'kill_stub; rm -rf "$stub_dir"' EXIT

# start_stub <script-file> -> sets $stub_port
start_stub() {
    "$stub_bin" "$1" > "$stub_dir/out" 2>"$stub_dir/err" &
    stub_pid=$!
    stub_port=""
    for _ in $(seq 1 100); do
        stub_port="$(sed -n 's/^listening //p' "$stub_dir/out" 2>/dev/null)"
        [ -n "$stub_port" ] && return 0
        kill -0 "$stub_pid" 2>/dev/null || break
        sleep 0.05
    done
    return 1
}

# builds a kabuk from a directory, entries sorted by byte order
mkkabuk() { # <dir> <out> <file>...
    d="$1"; o="$2"; shift 2
    {
        printf 'kabuk1\n'
        for f in "$@"; do
            printf '%s\n%d\n' "$f" "$(wc -c < "$d/$f")"
            cat "$d/$f"
        done
        printf 'sampura\n'
    } > "$o"
}

if [ -n "$stub_bin" ]; then
    pkg="$(mktemp -d)"
    mkdir -p "$pkg/src"
    cat > "$pkg/src/project.jsonc" <<'JSON'
{
    "project": {
        "name": "mylib",
        "version": "1.0.0",
        "entry": "main.oboe",
        "description": "a test library"
    }
}
JSON
    printf 'func hello() { return 7 }\n' > "$pkg/src/main.oboe"
    mkkabuk "$pkg/src" "$pkg/good.kabuk" main.oboe project.jsonc
    good_sema="$($OBOE sema "$pkg/good.kabuk" | cut -d' ' -f1)"
    good_len="$(wc -c < "$pkg/good.kabuk")"

    # an archive that tries to escape the package directory
    mkdir -p "$pkg/eviltree"
    printf 'pwned\n' > "$pkg/eviltree/evil"
    {
        printf 'kabuk1\n'
        printf '../evil\n%d\n' "$(wc -c < "$pkg/eviltree/evil")"
        cat "$pkg/eviltree/evil"
        printf 'sampura\n'
    } > "$pkg/evil.kabuk"
    evil_sema="$($OBOE sema "$pkg/evil.kabuk" | cut -d' ' -f1)"
    evil_len="$(wc -c < "$pkg/evil.kabuk")"

    newproj() { # -> $proj, an initialised project directory
        proj="$(mktemp -d)"
        ( cd "$proj" && "$OLDPWD/$OBOE" init >/dev/null 2>&1 )
    }

    # ---- a plain successful get ----
    cat > "$stub_dir/s1" <<SCRIPT
send dijabon katare/1 reedbed/0.1
expect dijabon katare/1 oboe/0.1
send si jexa cizujo kyx67108864
expect ko cizujo mylib *
send si kyx 79
sendbody $stub_dir/pins
expect ko besal mylib 1.0.0
send si kyx RECLEN
sendbody STUBDIR/rec
expectpre ko ghazema mylib 1.0.0
send si $good_sema kyx $good_len
sendbody $pkg/good.kabuk
read
SCRIPT
    printf 'mylib 1.0.0 %s\n' "$good_sema" > "$stub_dir/pins"
    # get asks what kind of package this is before fetching it
    {
        printf 'izim: mylib\nwaktanimra: 1.0.0\nwarna: vivlijotiki\n'
        printf 'ozhon: %s\nsema: %s\n' "$good_len" "$good_sema"
        printf 'wakta: 2026-07-14T09:21:00Z\n'
    } > "$stub_dir/rec"
    # body lengths have to match what is actually sent
    fixup() {
        sed -e "s|^send si kyx 79$|send si kyx $(wc -c < "$stub_dir/pins")|" \
            -e "s|RECLEN|$(wc -c < "$stub_dir/rec")|" \
            -e "s|STUBDIR|$stub_dir|" "$1" > "$1.fixed" && mv "$1.fixed" "$1"
    }
    fixup "$stub_dir/s1"

    if start_stub "$stub_dir/s1"; then
        newproj
        ( cd "$proj" && OBOE_REGISTRY="katare://127.0.0.1:$stub_port/" \
            "$OLDPWD/$OBOE" get mylib >/dev/null 2>&1 )
        if [ -f "$proj/.oboe/libraries/mylib/main.oboe" ] &&
           grep -q '"mylib"' "$proj/project.jsonc" &&
           grep -q "mylib 1.0.0 $good_sema" "$proj/.oboe/lock"; then
            echo "PASS get_installs_package"
            pass=$((pass+1))
        else
            echo "FAIL get_installs_package"
            fail=$((fail+1))
        fi
        rm -rf "$proj"
    else
        echo "FAIL get_installs_package (stub did not start)"
        fail=$((fail+1))
    fi
    kill_stub

    # ---- a digest that does not match the bytes ----
    cat > "$stub_dir/s2" <<SCRIPT
send dijabon katare/1 reedbed/0.1
expect dijabon katare/1 oboe/0.1
send si jexa cizujo kyx67108864
expect ko cizujo mylib *
send si kyx $(wc -c < "$stub_dir/pins")
sendbody $stub_dir/pins
expect ko besal mylib 1.0.0
send si kyx RECLEN
sendbody STUBDIR/rec
expectpre ko ghazema mylib 1.0.0
send si sha256:0000000000000000000000000000000000000000000000000000000000000000 kyx $good_len
sendbody $pkg/good.kabuk
read
SCRIPT
    fixup "$stub_dir/s2"
    if start_stub "$stub_dir/s2"; then
        newproj
        ( cd "$proj" && OBOE_REGISTRY="katare://127.0.0.1:$stub_port/" \
            "$OLDPWD/$OBOE" get mylib >/dev/null 2>&1 )
        rc=$?
        # nothing may be installed, and the manifest must be untouched
        if [ $rc -ne 0 ] && [ ! -e "$proj/.oboe/libraries/mylib" ] &&
           ! grep -q '"mylib"' "$proj/project.jsonc"; then
            echo "PASS get_rejects_bad_digest"
            pass=$((pass+1))
        else
            echo "FAIL get_rejects_bad_digest"
            fail=$((fail+1))
        fi
        rm -rf "$proj"
    else
        echo "FAIL get_rejects_bad_digest (stub did not start)"
        fail=$((fail+1))
    fi
    kill_stub

    # ---- an archive with a path that escapes the destination ----
    printf 'mylib 1.0.0 %s\n' "$evil_sema" > "$stub_dir/evilpins"
    cat > "$stub_dir/s3" <<SCRIPT
send dijabon katare/1 reedbed/0.1
expect dijabon katare/1 oboe/0.1
send si jexa cizujo kyx67108864
expect ko cizujo mylib *
send si kyx $(wc -c < "$stub_dir/evilpins")
sendbody $stub_dir/evilpins
expect ko besal mylib 1.0.0
send si kyx RECLEN
sendbody STUBDIR/rec
expectpre ko ghazema mylib 1.0.0
send si $evil_sema kyx $evil_len
sendbody $pkg/evil.kabuk
read
SCRIPT
    fixup "$stub_dir/s3"
    if start_stub "$stub_dir/s3"; then
        newproj
        canary="$proj/../evil"
        rm -f "$canary"
        ( cd "$proj" && OBOE_REGISTRY="katare://127.0.0.1:$stub_port/" \
            "$OLDPWD/$OBOE" get mylib >/dev/null 2>&1 )
        rc=$?
        if [ $rc -ne 0 ] && [ ! -e "$canary" ] &&
           [ ! -e "$proj/.oboe/libraries/mylib" ]; then
            echo "PASS get_rejects_path_traversal"
            pass=$((pass+1))
        else
            echo "FAIL get_rejects_path_traversal"
            fail=$((fail+1))
        fi
        rm -f "$canary"
        rm -rf "$proj"
    else
        echo "FAIL get_rejects_path_traversal (stub did not start)"
        fail=$((fail+1))
    fi
    kill_stub

    # ---- a status the protocol does not define is fatal ----
    cat > "$stub_dir/s4" <<'SCRIPT'
send dijabon katare/1 reedbed/0.1
expect dijabon katare/1 oboe/0.1
send si jexa cizujo kyx67108864
expect ko cizujo mylib *
send qqqq
read
SCRIPT
    if start_stub "$stub_dir/s4"; then
        newproj
        out="$( cd "$proj" && OBOE_REGISTRY="katare://127.0.0.1:$stub_port/" \
            "$OLDPWD/$OBOE" get mylib 2>&1 )"
        rc=$?
        if [ $rc -ne 0 ]; then
            echo "PASS get_unknown_status_is_fatal"
            pass=$((pass+1))
        else
            echo "FAIL get_unknown_status_is_fatal ($out)"
            fail=$((fail+1))
        fi
        rm -rf "$proj"
    else
        echo "FAIL get_unknown_status_is_fatal (stub did not start)"
        fail=$((fail+1))
    fi
    kill_stub

    # ---- a greeting from something that is not a katare/1 server ----
    cat > "$stub_dir/s5" <<'SCRIPT'
send dijabon katare/9 reedbed/0.1
read
SCRIPT
    if start_stub "$stub_dir/s5"; then
        newproj
        out="$( cd "$proj" && OBOE_REGISTRY="katare://127.0.0.1:$stub_port/" \
            "$OLDPWD/$OBOE" get mylib 2>&1 )"
        rc=$?
        if [ $rc -ne 0 ] && printf '%s' "$out" | grep -q 'katare/1'; then
            echo "PASS get_rejects_wrong_protocol_version"
            pass=$((pass+1))
        else
            echo "FAIL get_rejects_wrong_protocol_version ($out)"
            fail=$((fail+1))
        fi
        rm -rf "$proj"
    else
        echo "FAIL get_rejects_wrong_protocol_version (stub did not start)"
        fail=$((fail+1))
    fi
    kill_stub

    # ---- tidy is offline when the lockfile is already satisfied ----
    # No stub is running at all: if tidy dials out here, it fails.
    newproj
    mkdir -p "$proj/.oboe/libraries/mylib"
    cp "$pkg/src/project.jsonc" "$proj/.oboe/libraries/mylib/"
    cp "$pkg/src/main.oboe" "$proj/.oboe/libraries/mylib/"
    printf 'mylib 1.0.0 %s\n' "$good_sema" > "$proj/.oboe/lock"
    ( cd "$proj" && OBOE_REGISTRY="katare://127.0.0.1:1/" \
        "$OLDPWD/$OBOE" tidy >/dev/null 2>&1 )
    if [ $? -eq 0 ]; then
        echo "PASS tidy_offline_when_satisfied"
        pass=$((pass+1))
    else
        echo "FAIL tidy_offline_when_satisfied"
        fail=$((fail+1))
    fi
    rm -rf "$proj"


    # ---- the kango exchange ----
    cat > "$stub_dir/s6" <<'SCRIPT'
send dijabon katare/1 reedbed/0.1
expect dijabon katare/1 oboe/0.1
send si jexa cizujo kalit kango kaldy kyx67108864
expect kalit testtoken0000000000
send si robin
expectpre kango shapes 1.0.0 sha256:
send si
read
SCRIPT
    if start_stub "$stub_dir/s6"; then
        pubdir="$(mktemp -d)"
        cat > "$pubdir/project.jsonc" <<'JSON'
{
    "project": {
        "name": "shapes",
        "version": "1.0.0",
        "entry": "main.oboe",
        "description": "shapes"
    }
}
JSON
        printf 'func area(int w, int h) { return w * h }\n' > "$pubdir/main.oboe"
        ( cd "$pubdir" && OBOE_REGISTRY="katare://127.0.0.1:$stub_port/" \
            OBOE_TOKEN=testtoken0000000000 \
            "$OLDPWD/$OBOE" publish >/dev/null 2>&1 )
        rc=$?
        wait_rc=0
        wait "$stub_pid" 2>/dev/null || wait_rc=$?
        stub_pid=""
        # the stub asserts the exact kalit and the shape of the kango line, so
        # its own exit status is half of this assertion
        if [ $rc -eq 0 ] && [ $wait_rc -eq 0 ]; then
            echo "PASS publish_sends_kango"
            pass=$((pass+1))
        else
            echo "FAIL publish_sends_kango (oboe=$rc stub=$wait_rc)"
            fail=$((fail+1))
        fi
        rm -rf "$pubdir"
    else
        echo "FAIL publish_sends_kango (stub did not start)"
        fail=$((fail+1))
    fi
    kill_stub

    rm -rf "$pkg"
fi

# ---- publish ------------------------------------------------------------
#
# Packing is offline and deterministic, so most of this needs no server.

newpub() { # -> $pub, a publishable project
    pub="$(mktemp -d)"
    cat > "$pub/project.jsonc" <<'JSON'
{
    "project": {
        "name": "shapes",
        "version": "1.0.0",
        "entry": "main.oboe",
        "description": "shapes"
    }
}
JSON
    printf 'func area(int w, int h) { return w * h }\n' > "$pub/main.oboe"
}

newpub
a="$( cd "$pub" && "$OLDPWD/$OBOE" publish --dry-run 2>/dev/null | tail -1 )"
b="$( cd "$pub" && "$OLDPWD/$OBOE" publish --dry-run 2>/dev/null | tail -1 )"
if [ -n "$a" ] && [ "$a" = "$b" ]; then
    echo "PASS publish_dry_run_is_deterministic"
    pass=$((pass+1))
else
    echo "FAIL publish_dry_run_is_deterministic"
    fail=$((fail+1))
fi

# Build output and editor droppings must not change the digest, or a rebuild
# from a clean checkout would not reproduce what was published.
mkdir -p "$pub/dist" "$pub/.git"
echo junk > "$pub/dist/leftover"
echo x > "$pub/.git/config"
touch "$pub/main.o" "$pub/.DS_Store"
c="$( cd "$pub" && "$OLDPWD/$OBOE" publish --dry-run 2>/dev/null | tail -1 )"
if [ "$a" = "$c" ]; then
    echo "PASS publish_excludes_build_artifacts"
    pass=$((pass+1))
else
    echo "FAIL publish_excludes_build_artifacts"
    fail=$((fail+1))
fi

# .oboeignore is the configurable half of that
printf 'notes.md\n' > "$pub/.oboeignore"
printf 'notes\n' > "$pub/notes.md"
d="$( cd "$pub" && "$OLDPWD/$OBOE" publish --dry-run 2>/dev/null )"
if ! printf '%s' "$d" | grep -q 'notes[.]md' &&
   printf '%s' "$d" | grep -q 'oboeignore'; then
    echo "PASS publish_honours_oboeignore"
    pass=$((pass+1))
else
    echo "FAIL publish_honours_oboeignore"
    fail=$((fail+1))
fi

# `dist` is build output at the top level, but a package may have a src/dist/ of
# its own. Excluding it at any depth would publish an archive that is valid,
# reproducible and missing source -- the worst shape a bug can take here.
mkdir -p "$pub/src/dist"
printf 'func draw() { return 1 }\n' > "$pub/src/dist/renderer.oboe"
e="$( cd "$pub" && "$OLDPWD/$OBOE" publish --dry-run 2>/dev/null )"
if printf '%s' "$e" | grep -q 'src/dist/renderer[.]oboe'; then
    echo "PASS publish_keeps_nested_dist"
    pass=$((pass+1))
else
    echo "FAIL publish_keeps_nested_dist"
    fail=$((fail+1))
fi

# ... while the top-level one is still excluded
if ! printf '%s' "$e" | grep -q 'dist/leftover'; then
    echo "PASS publish_excludes_toplevel_dist"
    pass=$((pass+1))
else
    echo "FAIL publish_excludes_toplevel_dist"
    fail=$((fail+1))
fi

# A trailing slash means "directories only", as in .gitignore. Without support
# for it the idiom matches nothing and appears to work, because .oboeignore
# itself joins the archive and the digest changes anyway.
printf 'src/dist/\n' > "$pub/.oboeignore"
f="$( cd "$pub" && "$OLDPWD/$OBOE" publish --dry-run 2>/dev/null )"
if ! printf '%s' "$f" | grep -q 'renderer[.]oboe'; then
    echo "PASS publish_oboeignore_directory_pattern"
    pass=$((pass+1))
else
    echo "FAIL publish_oboeignore_directory_pattern"
    fail=$((fail+1))
fi
rm -rf "$pub"

# A kabuk cannot express a symlink, so publishing one is refused rather than
# quietly followed.
newpub
ln -s main.oboe "$pub/alias.oboe"
if ! ( cd "$pub" && "$OLDPWD/$OBOE" publish --dry-run >/dev/null 2>&1 ); then
    echo "PASS publish_refuses_symlink"
    pass=$((pass+1))
else
    echo "FAIL publish_refuses_symlink"
    fail=$((fail+1))
fi
rm -rf "$pub"

# A package name has to be an Oboe identifier, because `import foo` uses it.
newpub
sed 's/"name": "shapes"/"name": "Shapes-1"/' "$pub/project.jsonc" > "$pub/p2" &&
    mv "$pub/p2" "$pub/project.jsonc"
if ! ( cd "$pub" && "$OLDPWD/$OBOE" publish --dry-run >/dev/null 2>&1 ); then
    echo "PASS publish_rejects_bad_name"
    pass=$((pass+1))
else
    echo "FAIL publish_rejects_bad_name"
    fail=$((fail+1))
fi
rm -rf "$pub"

# ---- vendored wire-format code ------------------------------------------
#
# common/ lives in the reedbed repository; these are copies. Checking them
# against the manifest catches the usual failure -- editing a copy in place --
# here, without needing the other repository checked out.

if [ -f src/VENDOR.sha256 ]; then
    if ( cd src && "$OLDPWD/$OBOE" sema sha256.c sha256.h izim.c izim.h \
            record.c record.h kabuk.c kabuk.h projectjson.c projectjson.h |
            diff -u VENDOR.sha256 - >/dev/null ); then
        echo "PASS vendor_manifest"
        pass=$((pass+1))
    else
        echo "FAIL vendor_manifest (re-vendor from reedbed common/)"
        fail=$((fail+1))
    fi
else
    echo "SKIP vendor_manifest (no src/VENDOR.sha256)"
    skip=$((skip+1))
fi

# ---- sha256 known-answer vectors ----------------------------------------

kat_ok=1
[ "$(printf '' | $OBOE sema - | cut -d' ' -f1)" = \
  "sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" ] || kat_ok=0
[ "$(printf 'abc' | $OBOE sema - | cut -d' ' -f1)" = \
  "sha256:ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" ] || kat_ok=0
if [ $kat_ok -eq 1 ]; then
    echo "PASS sha256_kat"
    pass=$((pass+1))
else
    echo "FAIL sha256_kat"
    fail=$((fail+1))
fi

echo
echo "$pass passed, $fail failed, $skip skipped"
[ $fail -eq 0 ]
