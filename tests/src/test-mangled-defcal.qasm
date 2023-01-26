OPENQASM 3.0;

defcalgrammar "openpulse";

qubit $0;
qubit $1;
qubit $2;
qubit[8] $3;

uint n = 64U;

defcal rz(theta, lambda, phi) $1, $2 { }

defcal rx(theta) $3[0] { }

