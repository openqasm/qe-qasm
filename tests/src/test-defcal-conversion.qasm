OPENQASM 3.0;

float[64] sigma = 3.141592;
float[64] psi = 6.283184;

qubit $0;
qubit $1;
qubit $2;

defcal rx(pi / 2, psi) $0 { }

defcal ry(-pi / 2, sigma) $0 { }

defcal rz(pi / 4, -pi * 2, sigma, psi) $0, $1 { }

