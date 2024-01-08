OPENQASM 3.0;

gate x q { }

gate y q { }

qubit $0;
qubit $1;
qubit $2;
qubit $3;
qubit $4;
qubit $5;

int i = 17;

if (i == 0) {
  x $0;
  y $0;
} else if (i == 1) {
  x $1;
  y $1;
} else if (i == 2) {
  x $2;
  y $2;
} else if (i == 3) {
  x $3;
  y $3;
} else if (i == 4) {
  x $4;
  y $4;
} else {
  x $5;
  y $5;
}
