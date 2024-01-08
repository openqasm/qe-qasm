OPENQASM 3.0;

qubit[16] q;
bit[16] c;

c[0] = measure q[2];
c[1] = measure q[1];
c[2] = measure q[0];

c[4:7] = measure q[8:11];
c[10:8] = measure q[3:5];
