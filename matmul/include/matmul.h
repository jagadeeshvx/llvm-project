#ifndef MATMUL_H
#define MATMUL_H

// Square row-major float matmul: C = A * B, all matrices are n x n.
// Every variant overwrites C and computes the same result.

namespace matmul {

// Loop tile edge used by the tiled variants.
constexpr int kTile = 64;

// Baseline i-j-k loop order.
void naive(const float *A, const float *B, float *C, int n);

// Loop interchange: i-k-j order, unit-stride inner loop.
void interchange(const float *A, const float *B, float *C, int n);

// Interchange + loop tiling (kTile x kTile blocks).
void tiled(const float *A, const float *B, float *C, int n);

// Interchange + tiling + loop unrolling (k by 4, j by 4).
void tiled_unrolled(const float *A, const float *B, float *C, int n);

} // namespace matmul

#endif // MATMUL_H
