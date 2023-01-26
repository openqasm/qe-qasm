OPENQASM 3.0;

qubit $0;

bit result;

result = measure $0;

if (result==1) {
    U(pi, 0, pi) $0;
}
