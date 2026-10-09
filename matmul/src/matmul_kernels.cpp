#include "matmul.h"

#include <algorithm>

namespace matmul {

// 1. Baseline: i-j-k. B is walked column-wise (stride n) in the inner loop.
void naive(const float *A, const float *B, float *C, int n) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) {
      float sum = 0.0f;
      for (int k = 0; k < n; k++)
        sum += A[i * n + k] * B[k * n + j];
      C[i * n + j] = sum;
    }
}

// 2. Loop interchange: i-k-j. The inner loop walks B and C row-wise
//    (unit stride) and A[i][k] is loop-invariant.
void interchange(const float *A, const float *B, float *C, int n) {
  std::fill(C, C + n * n, 0.0f);
  for (int i = 0; i < n; i++)
    for (int k = 0; k < n; k++) {
      float a = A[i * n + k];
      for (int j = 0; j < n; j++)
        C[i * n + j] += a * B[k * n + j];
    }
}

// 3. Interchange + tiling: work on kTile x kTile blocks so the blocks of
//    B and C stay in cache while they are reused.
void tiled(const float *A, const float *B, float *C, int n) {
  std::fill(C, C + n * n, 0.0f);
  for (int ii = 0; ii < n; ii += kTile)
    for (int kk = 0; kk < n; kk += kTile)
      for (int jj = 0; jj < n; jj += kTile) {
        int iMax = std::min(ii + kTile, n);
        int kMax = std::min(kk + kTile, n);
        int jMax = std::min(jj + kTile, n);
        for (int i = ii; i < iMax; i++)
          for (int k = kk; k < kMax; k++) {
            float a = A[i * n + k];
            for (int j = jj; j < jMax; j++)
              C[i * n + j] += a * B[k * n + j];
          }
      }
}

// 4. Interchange + tiling + unrolling: k unrolled by 4 (four rows of B are
//    combined into one pass over C, cutting loads/stores of C by 4x) and j
//    unrolled by 4, each with a scalar remainder loop.
void tiled_unrolled(const float *A, const float *B, float *C, int n) {
  std::fill(C, C + n * n, 0.0f);
  for (int ii = 0; ii < n; ii += kTile)
    for (int kk = 0; kk < n; kk += kTile)
      for (int jj = 0; jj < n; jj += kTile) {
        int iMax = std::min(ii + kTile, n);
        int kMax = std::min(kk + kTile, n);
        int jMax = std::min(jj + kTile, n);
        for (int i = ii; i < iMax; i++) {
          float *c = &C[i * n];
          int k = kk;
          for (; k + 4 <= kMax; k += 4) {
            float a0 = A[i * n + k], a1 = A[i * n + k + 1];
            float a2 = A[i * n + k + 2], a3 = A[i * n + k + 3];
            const float *b0 = &B[k * n], *b1 = b0 + n, *b2 = b1 + n,
                        *b3 = b2 + n;
            int j = jj;
            for (; j + 4 <= jMax; j += 4) {
              c[j]     += a0 * b0[j]     + a1 * b1[j]     + a2 * b2[j]     + a3 * b3[j];
              c[j + 1] += a0 * b0[j + 1] + a1 * b1[j + 1] + a2 * b2[j + 1] + a3 * b3[j + 1];
              c[j + 2] += a0 * b0[j + 2] + a1 * b1[j + 2] + a2 * b2[j + 2] + a3 * b3[j + 2];
              c[j + 3] += a0 * b0[j + 3] + a1 * b1[j + 3] + a2 * b2[j + 3] + a3 * b3[j + 3];
            }
            for (; j < jMax; j++)
              c[j] += a0 * b0[j] + a1 * b1[j] + a2 * b2[j] + a3 * b3[j];
          }
          for (; k < kMax; k++) {
            float a = A[i * n + k];
            for (int j = jj; j < jMax; j++)
              c[j] += a * B[k * n + j];
          }
        }
      }
}

} // namespace matmul
