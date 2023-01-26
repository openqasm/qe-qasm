OPENQASM 3.0;

def bar(qubit j, qubit k, angle[32] y) -> bit {
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

bar($0, $1, pi / 2);

bar($0, $1, -pi / 2);

