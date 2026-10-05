// Strength reduction: multiply by power of 2 -> left shift
// EXPECT-PRESENT: shl i32 %[a-z0-9.]+, 1
// EXPECT-PRESENT: shl i32 %[a-z0-9.]+, 3
// EXPECT-ABSENT: mul
int sr(int c, int f) {
  int d = 2 * c; // c << 1
  int e = f * 8; // f << 3
  return d + e;
}
int main(int argc, char **argv) { return sr(argc + 2, argc + 3); }
