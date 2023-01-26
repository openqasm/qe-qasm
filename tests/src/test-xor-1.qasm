OPENQASM 3.0;

uint[64] c;

// 1. XOr assign.
c ^= 1;

uint zx = 3;

// 2. Xor assign uint.
uint zy = zx ^= 1;

uint n = 3;

uint q = zx + n;

// 3. Xor assign identifier.
c ^= zx;

