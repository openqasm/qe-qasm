OPENQASM 3.0;


def foo(int i, qubit[8] d) -> bit {
  return measure d[i];
}

int i = 15;

int j = 1;
int k = 2;

float[64] d = 0.0;

bit c1;

qubit[8] $0;
qubit $1;

switch (i) {
  case 1: {
    j = k + foo(k, $0);
  }
  break;
  case 2:
    d = j / k;
  break;
  case 3:
  case 5:
    c1 = foo(k, $0);
  break;
  default:
  break;
}

