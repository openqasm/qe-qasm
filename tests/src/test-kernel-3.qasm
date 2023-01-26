OPENQASM 3.0;

uint m = 64;

uint n = 128;

// 1. Extern declaration with no statements and mixed parameter types.
extern test_kernel_1(qubit[4] d, qubit a, complex[float[n]] c, int i) -> bit;

// 2. Another extern declaration.
extern test_kernel_2(int a, int[m] b, float c, float[n] d) -> int;

