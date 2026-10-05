# custompass — out-of-tree LLVM optimization pass

Lives in `llvm-project/llvm/lib/Transforms/custompass/`. Pass name: `custom-opt`.

Steps (all written from scratch, iterated to a fixed point):
1. Algebraic simplification — `a/a→1`, `b-b→0`, `x*1→x`, `x+0→x` ...
2. Constant folding — `2+3→5`
3. Strength reduction — `x*8→x<<3`, unsigned `x/4→x>>2`
4. CSE (within a basic block)
5. Dead code elimination

```
custompass/
├── CMakeLists.txt       add_llvm_pass_plugin(CustomOptPass ...)
├── src/                 pass sources (one file per optimization)
├── Sample-test-files/   C inputs; each has // EXPECT-PRESENT / EXPECT-ABSENT checks
├── run_tests.sh         build-free test runner (IR checks + runtime equivalence)
└── run_pass.sh          run the pass on arbitrary .c/.cpp files
```

## Build (part of the normal LLVM build)

`lib/Transforms/CMakeLists.txt` has `add_subdirectory(custompass)`, and
`custompass/CMakeLists.txt` uses `add_llvm_pass_plugin`. From your LLVM build dir:

```bash
cd ~/jagadeesh/tvm/build
cmake .                      # re-run configure once to pick up the new folder
ninja CustomOptPass          # -> build/lib/CustomOptPass.so
ninja clang opt              # if not already built
```

## Verify

```bash
cd ~/jagadeesh/tvm/llvm-project/llvm/lib/Transforms/custompass
./run_tests.sh          # expect: == 6 passed, 0 failed ==
```
For each sample it compiles with the LLVM build's clang, runs
`opt -passes="mem2reg,custom-opt"`, checks the optimized IR, and checks the
optimized program returns the same exit code as the original.
Artifacts are written to `out/`.

## Run by hand

```bash
B=$HOME/jagadeesh/tvm/build/bin
$B/clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm Sample-test-files/Algebraic.c -o out/a.ll
$B/opt -load-pass-plugin=$HOME/jagadeesh/tvm/build/lib/CustomOptPass.so -passes="mem2reg,custom-opt" -S out/a.ll -o out/a.opt.ll
```
**`mem2reg` must run first**: -O0 IR keeps variables in memory, which this
pass doesn't look through. Use the same LLVM build's `clang`/`opt` that the
plugin was compiled against. Set `LLVM_BUILD=...` if your build dir differs.

## Limits
Integer arithmetic only; no signed-division reduction; CSE is per basic block.
