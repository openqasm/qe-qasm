OPENQASM 3.0;

// 1. Kernel declaration with no statements.
extern test_kernel_1(qubit[4] d, qubit a) -> bit;

uint n = 100;

// 2. Kernel declaration with statements.
extern test_kernel_2(bit[n] cin, qubit a) -> bit { }

// 3. Kernel declaration with classical arguments.
extern test_kernel_3(int i, float[32] f) -> float[32];

// 4. Kernel declaration with no statements and mixed parameter types.
extern test_kernel_4(qubit[4] d, qubit a, int[128] i, float[32] f) -> bit;

// 5. Kernel declaration with statements and mixed parameter types.
extern test_kernel_5(qubit[4] d, qubit a, int[128] i, float[128] f) -> bit[8] { }
