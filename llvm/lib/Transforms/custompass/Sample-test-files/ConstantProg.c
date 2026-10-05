// Constant / copy propagation
// EXPECT-PRESENT: ret i32 9
// EXPECT-ABSENT: add
int main() {
  int b = 4, c, d = 5, e;
  c = d;     // copy propagation
  e = c + b; // 9
  return e;
}
