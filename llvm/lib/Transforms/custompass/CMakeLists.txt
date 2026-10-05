# Out-of-line custom optimization pass plugin ("custom-opt"), built as part
# of the normal LLVM build. Produces <build>/lib/CustomOptPass.so.
#
#   ninja -C <build> CustomOptPass
#   opt -load-pass-plugin=<build>/lib/CustomOptPass.so -passes="mem2reg,custom-opt" ...
add_llvm_pass_plugin(CustomOptPass
  src/CustomOptPass.cpp
  src/Common.cpp
  src/AlgebraicSimplify.cpp
  src/ConstantFold.cpp
  src/StrengthReduction.cpp
  src/CommonSubexpressionElimination.cpp
  src/DeadCodeElimination.cpp
  )
