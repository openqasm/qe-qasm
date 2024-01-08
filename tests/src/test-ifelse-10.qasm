OPENQASM 3.0;

gate H q { }

gate X q { }

gate Z q { }

bit c0;
bit c1;

qubit $0;
qubit $1;

int i = 3;
int j = 4;

c0 = measure $0;
c1 = measure $1;

if (c0 == i) {
  H $0;
  H $0;
  H $0;
} else if (c0 > i) {
  Z $0;
  Z $0;
  Z $0;
  if (c1 == j) {
    H $1;
    Z $1;
    X $1;
  } else if (c1 > j) {
    X $1;
    Z $1;
    H $1;
  } else if (c1 == c0) {
    Z $0;
    X $0;
    Z $0;
  } else {
    Z $0;
    Z $1;
  }
} else {
  X $0;
  X $0;
  X $0;
}
