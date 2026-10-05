#!/usr/bin/env bash
# Builds nothing; runs the custom pass on every Sample-test-files/*.c and checks:
#   1. IR assertions written in each file as
#        // EXPECT-PRESENT: <regex>   (must match somewhere in optimized IR)
#        // EXPECT-ABSENT:  <regex>   (must NOT match in optimized IR)
#   2. the optimized program returns the same exit code as the original.
# Also asserts that the optimized IR has fewer instructions than the input.
#
# Usage: ./run_tests.sh [files...]     env: LLVM_BUILD=<llvm build dir>
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLVM_BUILD="${LLVM_BUILD:-$HERE/../../../../../build}"
CLANG="$LLVM_BUILD/bin/clang"; OPT="$LLVM_BUILD/bin/opt"
PLUGIN="${PLUGIN:-$LLVM_BUILD/lib/CustomOptPass.so}"
OUT="$HERE/out"; mkdir -p "$OUT"
for t in "$CLANG" "$OPT" "$PLUGIN"; do
  [[ -e "$t" ]] || { echo "missing: $t (build the plugin first, see README)"; exit 2; }
done
FILES=("$@"); [[ ${#FILES[@]} -gt 0 ]] || FILES=("$HERE"/Sample-test-files/*.c)
pass=0; fail=0
for f in "${FILES[@]}"; do
  n="$(basename "${f%.c}")"; ok=1; msgs=""
  "$CLANG" -O0 -Xclang -disable-O0-optnone -S -emit-llvm "$f" -o "$OUT/$n.ll" 2>/dev/null || { echo "FAIL $n (clang)"; fail=$((fail+1)); continue; }
  "$OPT" -load-pass-plugin="$PLUGIN" -passes="mem2reg,custom-opt" -S "$OUT/$n.ll" -o "$OUT/$n.opt.ll" 2>"$OUT/$n.log" || { echo "FAIL $n (opt)"; fail=$((fail+1)); continue; }
  body="$(grep -vE '^(;|!|attributes|source_filename|target|declare)' "$OUT/$n.opt.ll")"
  while IFS= read -r re; do
    grep -Eq "$re" <<<"$body" || { ok=0; msgs+="\n    expected present: $re"; }
  done < <(sed -n 's#^// EXPECT-PRESENT: ##p' "$f")
  while IFS= read -r re; do
    grep -Eq "$re" <<<"$body" && { ok=0; msgs+="\n    expected absent: $re"; }
  done < <(sed -n 's#^// EXPECT-ABSENT: ##p' "$f")
  b="$(grep -cE '^  ' "$OUT/$n.ll")"; a="$(grep -cE '^  ' "$OUT/$n.opt.ll")"
  (( a < b )) || { ok=0; msgs+="\n    no instructions removed ($b -> $a)"; }
  "$CLANG" "$OUT/$n.ll" -o "$OUT/$n.orig" 2>/dev/null; "$OUT/$n.orig"; r1=$?
  "$CLANG" "$OUT/$n.opt.ll" -o "$OUT/$n.opt" 2>/dev/null; "$OUT/$n.opt"; r2=$?
  (( r1 == r2 )) || { ok=0; msgs+="\n    exit code differs: original=$r1 optimized=$r2"; }
  if (( ok )); then echo "PASS $n  (instrs $b -> $a, exit=$r1)"; pass=$((pass+1))
  else echo -e "FAIL $n$msgs"; fail=$((fail+1)); fi
done
echo "== $pass passed, $fail failed =="; exit $((fail>0))
