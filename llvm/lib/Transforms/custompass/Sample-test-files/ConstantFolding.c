// Constant folding
// EXPECT-PRESENT: ret i32 22
// EXPECT-ABSENT: add|mul|sdiv
int compute() {
  int result = 0;
  int a = 2;
  int b = 3;
  int c = 4 + a + b; // 9
  result += a;       // 2
  result += b;       // 5
  result *= c;       // 45
  result /= 2;       // 22
  a = result;
  return result;
}
int main() { return compute(); }
