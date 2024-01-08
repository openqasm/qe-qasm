OPENQASM 3.0;

extern test5(int[32] a) -> int[64];

int[32] e = 5;
int[64] f;

f = test5(e);
