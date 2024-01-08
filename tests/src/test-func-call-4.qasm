OPENQASM 3.0;

def baz(qubit j, qubit k) -> bit {
  bit r = 0;

  reset j;
  reset k;

  U(pi, 0, pi) j;

  r = measure j;

  return r;
}

qubit $0;
qubit $1;

bit c2 = 0;

baz($0, $1);
