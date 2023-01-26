OPENQASM 3.0;

float q = 13.5;

uint j;

int k;

uint n;

int u;

j = q % 3;

k = 17 % 4;

n = 32 % j;

u = q % n;

qubit %0;
qubit %1;

reset %0;
reset %1;

