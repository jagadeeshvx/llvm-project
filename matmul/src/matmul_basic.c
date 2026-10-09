#include <stdio.h>

#define M 4
#define K 3
#define N 5

// C[M][N] = A[M][K] * B[K][N]
void matmul(const float A[M][K], const float B[K][N], float C[M][N]) {
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      float sum = 0.0f;
      for (int k = 0; k < K; k++) {
        sum += A[i][k] * B[k][j];
      }
      C[i][j] = sum;
    }
  }
}

int main(void) {
  float A[M][K], B[K][N], C[M][N];

  for (int i = 0; i < M; i++)
    for (int k = 0; k < K; k++)
      A[i][k] = (float)(i + k);

  for (int k = 0; k < K; k++)
    for (int j = 0; j < N; j++)
      B[k][j] = (float)(k * j + 1);

  matmul(A, B, C);

  printf("C = A * B:\n");
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%8.2f ", C[i][j]);
    printf("\n");
  }
  return 0;
}
