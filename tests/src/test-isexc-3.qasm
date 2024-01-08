OPENQASM 3.0;

gate X q { }

qubit $1;

bit is_excited;

if (is_excited) {
  if (is_excited) {
    U $1;
    U $1;
    U $1;
  } else {
    X $1;
    X $1;
    X $1;
  }

  H $1;
  H $1;
  H $1;
}
