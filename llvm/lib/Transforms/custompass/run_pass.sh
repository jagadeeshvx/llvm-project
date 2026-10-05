#!/usr/bin/env bash
# Batch-apply CustomOptPass to every .cpp file passed on the command line
# (or every *.cpp in the current directory if none are given).
#
# Usage:
#   ./run_pass.sh file1.cpp file2.cpp ...
#   ./run_pass.sh            # processes all *.cpp in cwd
#
# Requires: clang++, opt (same LLVM install/version the plugin was built
# against), and libCustomOptPass.so built in ./build (see README.md).

set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLVM_BUILD="${LLVM_BUILD:-$HERE/../../../../../build}"   # tvm/build
CLANG="$LLVM_BUILD/bin/clang"
OPT="$LLVM_BUILD/bin/opt"
PLUGIN="${PLUGIN:-$LLVM_BUILD/lib/CustomOptPass.so}"
[[ -f "$PLUGIN" ]] || PLUGIN="$HERE/build/libCustomOptPass.dylib"  # macOS

if [[ ! -f "$PLUGIN" ]]; then
  echo "error: plugin not found. Build it first (see README.md)." >&2
  exit 1
fi

FILES=("$@")
if [[ ${#FILES[@]} -eq 0 ]]; then
  shopt -s nullglob
  FILES=(*.c *.cpp)
fi

if [[ ${#FILES[@]} -eq 0 ]]; then
  echo "No .cpp files given or found." >&2
  exit 1
fi

mkdir -p "$HERE/out"

for f in "${FILES[@]}"; do
  base="$(basename "${f%.*}")"
  ir="$HERE/out/${base}.ll"
  opt_ir="$HERE/out/${base}.opt.ll"
  bin="$HERE/out/${base}.opt"

  echo "== $f =="

  # -O0 -disable-O0-optnone: emit IR without the 'optnone' attribute that
  # would otherwise block opt from running any passes on it.
  "$CLANG" -O0 -Xclang -disable-O0-optnone -S -emit-llvm "$f" -o "$ir"

  # mem2reg first: -O0 IR keeps variables in memory (alloca/load/store),
  # which the custom pass does not look through.
  "$OPT" -load-pass-plugin="$PLUGIN" -passes="mem2reg,custom-opt" \
      -S "$ir" -o "$opt_ir"

  # Optional: compile the optimized IR to a runnable binary.
  "$CLANG" "$opt_ir" -o "$bin" || echo "  (skipped native link for $f)"

  echo "  wrote $opt_ir"
done

echo "Done. Optimized IR is in ./out/*.opt.ll"
