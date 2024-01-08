OPENQASM 3.0;

def foo(int i, qubit[8] d) -> bit {
  return measure d[i];
}

int k = 1;
int l = 2;

qubit[8] $0;
qubit $1;

bit c1;
bit c2;

c1 = foo(k + l - 5, $0);

def bar(float[32] f, angle[32] a, qubit j) -> bit {
}

float m = 4.5;

angle[32] phi = 7.5;

c2 = bar(m / 3, phi, $1);
