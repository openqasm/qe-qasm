OPENQASM 3.0;

qubit $2;
qubit $3;

bit is_excited;
bit other;
bit result;

gate x q {
   U(pi, 0, pi) q;
}

x $2;
x $3;

is_excited = measure $2;

if (is_excited == 1) {
  other = measure $3;
  if (other == 1){
     x $2;
  }
}

result = measure $2;
