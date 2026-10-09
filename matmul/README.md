# Matrix Multiplication: Making It Faster

This small project shows how three classic loop tricks speed up a matrix
multiplication written in plain C++. It starts with the simple three-loop
version and adds one idea at a time, so you can see what each one buys you.

## What's in the folder

| File | What it is |
|------|-----------|
| `src/matmul_basic.c` | The simple C version: three `for` loops on a tiny 4x3 by 3x5 example. Good for understanding the maths. |
| `include/matmul.h` | Lists the four versions of the multiply (and the tile size). |
| `src/matmul_kernels.cpp` | The four versions themselves. Read this file to see the changes. |
| `bench/benchmark.cpp` | Checks every version gives the right answer, then times them. |
| `Makefile` | Shortcuts for building and running. |

## The four versions

1. **Naive** – the textbook `i, j, k` loops. For each output cell it walks down
   a column of B, which jumps across memory and keeps missing the CPU cache.
2. **Loop interchange** – swaps the loops to `i, k, j`. Now the inner loop reads
   B and C straight along a row, so memory is read in order and the cache works
   well. It also lets the compiler use SIMD (several numbers per instruction).
3. **Loop tiling** – cuts the matrices into 64x64 blocks and finishes one block
   before moving on, so the data being reused stays in the cache.
4. **Loop unrolling** – on top of tiling, handles 4 values of `k` and 4 values
   of `j` per step. That means fewer loop checks and far fewer reads and writes
   of the result matrix.

## Setup

You need a C++17 compiler (`g++`) and `make`. Nothing else.

```
cd llvm-project/matmul
make run SIZE=1024
```

`SIZE` is the matrix width (default 512). To let the compiler optimise harder
and use your CPU's special instructions:

```
make clean
make OPT="-O3 -march=native" run SIZE=1024
```

Running plain `make` also builds the small C example at `build/matmul_basic`.
`make clean` removes everything that was built.

## Results

Test: two 1024 x 1024 float matrices, best of 5 runs. Higher speedup is better.
"Speedup" is measured against the naive version.

### Standard build (`-O2`)

| Version | Time | Speed (GFLOP/s) | Speedup |
|---------|-----:|----------------:|--------:|
| Naive | 1553 ms | 1.4 | 1.0x |
| + Loop interchange | 559 ms | 3.8 | 2.8x |
| + Tiling | 657 ms | 3.3 | 2.4x |
| + Tiling + unrolling | 347 ms | 6.2 | **4.5x** |

### Aggressive build (`-O3 -march=native`)

| Version | Time | Speed (GFLOP/s) | Speedup |
|---------|-----:|----------------:|--------:|
| Naive | 1481 ms | 1.5 | 1.0x |
| + Loop interchange | 140 ms | 15.4 | **10.6x** |
| + Tiling | 234 ms | 9.2 | 6.3x |
| + Tiling + unrolling | 173 ms | 12.4 | 8.6x |

(Numbers come from one machine and vary a little between runs. Expect the same
pattern on yours, not the exact figures.)

## What the numbers tell us

- **Loop interchange is the big win.** One simple change gives 2.8x on its own,
  and over 10x when the compiler is allowed to use SIMD.
- **Unrolling helps most in the standard build.** At `-O2` it nearly doubles the
  speed of the tiled version.
- **Tiling did not help at this size.** A 1024-wide row is only 4 KB, so the
  cache was already coping after interchange, and tiling added extra loop
  overhead. It should matter more for much larger matrices; try
  `make run SIZE=2048` to see.
- **With `-O3`, the compiler does a lot by itself.** It vectorises the simple
  interchanged loop so well that our hand-unrolled version is slightly slower.
  Hand tuning pays off most when the compiler is not doing it for you.

## Is the answer still right?

Yes. The benchmark compares every version with the naive result and prints
`OK` in the last column. Tiny differences are allowed because adding
floating-point numbers in a different order changes the last few digits.
