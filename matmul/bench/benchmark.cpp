// Correctness check + timing for every matmul variant.
// Usage: ./benchmark [size]   (default 512)
#include "matmul.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <vector>

namespace {

using Kernel = void (*)(const float *, const float *, float *, int);

struct Variant {
  const char *name;
  Kernel fn;
};

// Best-of-reps wall time in milliseconds.
double bestTimeMs(const std::function<void()> &fn, int reps) {
  double best = 1e30;
  for (int r = 0; r < reps; r++) {
    auto t0 = std::chrono::steady_clock::now();
    fn();
    auto t1 = std::chrono::steady_clock::now();
    best = std::min(best,
                    std::chrono::duration<double, std::milli>(t1 - t0).count());
  }
  return best;
}

float maxAbsDiff(const std::vector<float> &x, const std::vector<float> &y) {
  float m = 0;
  for (size_t i = 0; i < x.size(); i++)
    m = std::max(m, std::fabs(x[i] - y[i]));
  return m;
}

} // namespace

int main(int argc, char **argv) {
  const int n = argc > 1 ? std::atoi(argv[1]) : 512;
  const int reps = 5;

  std::vector<float> A(n * n), B(n * n), C(n * n), ref(n * n);
  for (int i = 0; i < n * n; i++) {
    A[i] = (float)(std::rand() % 10) / 10.0f;
    B[i] = (float)(std::rand() % 10) / 10.0f;
  }

  const Variant variants[] = {
      {"naive (i-j-k)", matmul::naive},
      {"interchange (i-k-j)", matmul::interchange},
      {"interchange + tiling", matmul::tiled},
      {"interchange + tiling + unroll", matmul::tiled_unrolled},
  };

  matmul::naive(A.data(), B.data(), ref.data(), n);
  const double flops = 2.0 * n * n * n;
  const float tol = 1e-3f * n; // float accumulation order differs per variant
  double baseMs = 0;

  printf("Matrix size: %d x %d, tile = %d\n", n, n, matmul::kTile);
  printf("%-32s %10s %10s %9s %8s\n", "variant", "time(ms)", "GFLOP/s",
         "speedup", "check");
  for (const Variant &v : variants) {
    double ms = bestTimeMs([&] { v.fn(A.data(), B.data(), C.data(), n); }, reps);
    bool ok = maxAbsDiff(C, ref) <= tol;
    if (baseMs == 0)
      baseMs = ms;
    printf("%-32s %10.2f %10.2f %8.2fx %8s\n", v.name, ms, flops / ms / 1e6,
           baseMs / ms, ok ? "OK" : "FAIL");
  }
  return 0;
}
