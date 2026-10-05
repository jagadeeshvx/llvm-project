// Common subexpression elimination
// EXPECT-PRESENT: mul
// EXPECT-ABSENT: add i32 %[a-z0-9.]+, 0
int cse(int x, int y) {
  int a = x * y + 1;
  int b = x * y + 2; // x*y repeated
  return a + b;
}
int main(int argc, char **argv) { return cse(argc + 2, argc + 3); }
