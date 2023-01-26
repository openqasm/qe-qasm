OPENQASM 3.0;

int a;
int b;
int c;
int d;

int i;
int j;
int k;

qubit $0;
qubit $1;
qubit $2;

gate H q { }

gate Z q { }

if (a == 3) {
  b = 9;
  H $0;
  H $0;
  H $0;
} else if (a > 3) {
  Z $1;
  Z $1;

  if (b == 2) {
    if (c > a) {
      H $2;
      Z $2;
      d *= 4;
    } else if (c < a) {
      d = c;
    } else {
      Z $0;
      Z $0;
      Z $0;
      Z $0;
    }
  } else {
    i = 1;
    j = 2;
    k = 3;
  }
}

