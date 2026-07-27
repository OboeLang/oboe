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

echo
echo "$pass passed, $fail failed"
[ $fail -eq 0 ]
