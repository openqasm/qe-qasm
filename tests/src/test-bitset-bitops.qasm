OPENQASM 3.0;

bit[2] a = 1;
bit[2] b = 0;
bit[2] c;

c = a << 1;
c = rotl(a, 2);
c = popcount(a);
c = a & b;
c = a | b;
c = a ^ b;

c = a + b;
c = a - b;
c = a * b;
c = a / b;

