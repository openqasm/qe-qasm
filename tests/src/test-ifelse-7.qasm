OPENQASM 3.0;

gate H a { U(π/2, 0, π) a; }

gate Z a { U(π/2, 0, π/4) a; }

bit qc0;

qubit $0;
qubit $1;

qc0 = measure $0;

if (qc0) {
  H $0;
  H $0;
  H $0;
  H $0;
} else {
  Z $0;
  Z $1;
  Z $0;
}
