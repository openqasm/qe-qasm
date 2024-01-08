OPENQASM 3.0;

// 1. Single Angle declaration. Default-initialized to 0.
angle[64] a;

// 2. Single Angle declaration with initialization - integer.
angle[64] b = 1;

// 3. Single Angle declaration with initialization - integer.
angle[128] bneg = -11;

// 4. Single Angle declaration with initialization - floating-point.
angle[64] fb = 5.197158;

// 5. Multiple Angle declarations. Default-initialized to 0.
angle[64] bw;
angle[64] by;

// 6. Multiple Angle declarations with initialization - integer.
angle[64] aw = 1;
angle[64] ay = 1;

// 7. Multiple Angle declarations with initialization - floating-point.
angle[64] fw = 3.14159;
angle[128] fy = 3.14159;

uint ug = 13;

// 8. Single Angle declaration with initialization - binary expression.
angle[64] ugf = ug << 2;
