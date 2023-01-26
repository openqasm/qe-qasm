OPENQASM 3.0;

// 1. Uninitialized uint declaration.
uint j;

// 2. Initialized uint declaration.
uint k = 4;

// 3. Uninitialized int declaration.
int g;

// 4. Initialized int declaration.
int i = 3;

// 5. Add self-assignment.
j += k;

// 6. uint Declaration + initialization from expression.
uint t = i << k;

// 7. Multiple-precision declaration + initialization from expression.
int[128] i8 = t ^ i;

// 8. Multiple-precision declaration + initialization from expression.
uint[96] v = i8 - i;

// 9. Multiple-precision declaration.
int[96] za;

// 9. Multiple-precision declaration.
uint[84] zb;

// 10. Multiple-precision declaration + initialization from expression.
uint[128] zc = za + zb;

// 11. Floating-point uninitialized declaration.
float d;

// 12. Floating-point declaration with initialization.
float notpi = 3.14159;

// 13. Multiple-precision floating-point uninitialized declaration.
float[256] mpda;

// 14. Multiple-precision floating-point declaration with initialization.
float[128] mpdb = pi / 2;

// 14. Multiple-precision floating-point declaration with initialization.
float[96] mpdc = 53.953953;

