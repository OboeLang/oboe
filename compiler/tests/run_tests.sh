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

for src in tests/*.oboe; do
    name="${src%.oboe}"
    base="$(basename "$name")"
    # helper modules (imported by other tests) start with _ and aren't run directly
    case "$base" in _*) continue ;; esac

    if [ -f "$name.expect_fail" ]; then
        out="$($OBOE run "$src" 2>&1)"
        rc=$?
        want="$(cat "$name.expect_fail")"
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

    expected="$name.expected"
    if [ ! -f "$expected" ]; then
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

echo
echo "$pass passed, $fail failed"
[ $fail -eq 0 ]
