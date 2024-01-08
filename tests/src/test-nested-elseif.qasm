OPENQASM 3.0;

gate x q { }

gate y q { }

qubit $0;
qubit $1;
qubit $2;
qubit $3;
qubit $4;
qubit $5;

int i;
int j;
int k;

if (i == 0) {
  x $0;
  y $0;
} else if (i == 1) {
  x $1;
  y $1;
  if (j == 2) {
    x $2;
    y $2;
  } else if (j == 3) {
    x $3;
    y $3;
  } else if (j == 5) {
    x $5;
    y $5;
  }
} else if (i == 2) {
  x $2;
  y $2;
} else if (i == 3) {
  x $3;
  y $3;
  if (k == 5) {
    x $1;
    y $1;
  } else if (k == 6) {
    x $2;
    y $2;
  } else if (k == 7) {
    x $3;
    y $3;
  } else if (k == 8) {
    x $4;
    y $4;
  } else {
    x $5;
    y $5;
  }
} else if (i == 4) {
  x $4;
  y $4;
} else {
  x $5;
  y $5;
}
