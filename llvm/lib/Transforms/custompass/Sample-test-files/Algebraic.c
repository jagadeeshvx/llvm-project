// Algebraic identity examples
// EXPECT-PRESENT: ret i32 23
// EXPECT-ABSENT: sdiv|sub |mul
int compute(int a, int b) {
  int result = (a / a);   // 1
  result *= (b / b);      // 1 * 1
  result += (b - b);      // 1 + 0
  result /= result;       // 1 / 1
  result -= result;       // 1 - 1
  result += 23;           // constant folding
  return result;
}
int main(int argc, char **argv) { return compute(argc + 4, argc + 6); }
