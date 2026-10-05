// Redundant assignment / dead code elimination
// EXPECT-PRESENT: ret i32 3
// EXPECT-ABSENT: add|alloca|store
int main() {
  int a = 3, b = 4, c;
  a = 3;
  b = 4;
  c = a + b; // c is never used
  return a;
}
