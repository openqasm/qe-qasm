OPENQASM 3.0;

qubit $1;

bit is_excited;

if (is_excited) {
  if (is_excited) {
    U $1;
    U $1;
    U $1;
  }

  H $1;
  H $1;
  H $1;
}
