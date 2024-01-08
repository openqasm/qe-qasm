OPENQASM 3.0;

include "stdgates.inc";

qubit $0;
qubit $1;
qubit $2;

bit c0;
bit c1;
bit c2;

reset $0;
reset $1;
reset $2;

U(0.3, 0.2, 0.1) $0;

h $0;
cx $1, $2;

duration for0 = 100ns;
duration for1 = 300ns;
duration for2 = 200ns;

// Need to define these
delay[for1] $0;
delay[for0] $1;
delay[for2] $2;

barrier $0, $1, $2;
cx $0, $1;
h $0;

c0 = measure $0;
c1 = measure $1;

if (c0 == 1) {
  z $2;
}

if (c1 == 1) {
  x $2;
}

c2 = measure $2;
