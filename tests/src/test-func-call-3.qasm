OPENQASM 3.0;

def baz(qubit j, qubit k) -> bit {
  reset j;
  reset k;

  U(pi, 0, pi) j;

  return measure k;
}

qubit $0;
qubit $1;

bit c2 = 0;

c2 = baz($0, $1);

