OPENQASM 3.0;

qubit $1;
qubit $2;
qubit[8] $3;

defcal measure $1 -> bit { }

defcal "openpulse" measure $2 -> bit { }

defcal measure $3 -> bit[8] { }

