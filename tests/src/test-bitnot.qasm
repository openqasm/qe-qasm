OPENQASM 3.0;

qubit $0;

gate x q {}

bit[4] c;

c[0] = 1;

// 1. BitNot assign
c[1] = ~c[0];

c[2] = measure $0;

// 2. BitNot condition
if (~c[2]) {
    x $0;
}
