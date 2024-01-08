OPENQASM 3.0;

gate H a { U(π/2, 0, π) a; }

gate Z a { U(π/2, 0, π/4) a; }

gate X q { }

bit qc0;
qubit $0;

int i = 3;
int j = 4;

qc0 = measure $0;

if (qc0 == i) {
  H $0;
  H $0;
  H $0;
} else {
  if (qc0 == j) {
    Z $0;
    Z $0;
    Z $0;
  } else {
    X $0;
    X $0;
    X $0;
  }
}
