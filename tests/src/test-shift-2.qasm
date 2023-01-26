OPENQASM 3.0;

angle[3] c = 0;

// 1. Right-shift assign.
c >>= 1;

uint zx = 3;

// 2. Right-shift assign uint.
uint zy = zx >>= 1;

