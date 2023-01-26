OPENQASM 3.0;

def foo(int i, qubit a, qubit b, qubit c) -> bit {
  if (i == 0)
    return measure a;
  else if (i == 1)
    return measure b;
  else
    return measure c;
}

int k = 1;
int l = 5;

qubit[8] $0;

bit c1;

c1 = foo(k + l - 3, $0[0], $0[1], $0[2]);

