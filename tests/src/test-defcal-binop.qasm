OPENQASM 3.0;

qubit $0;
qubit $1;
qubit $2;

defcal rx(pi / 2) $0 { }

defcal ry(-pi / 2) $0 { }

defcal rz(pi / 4, -pi * 2) $0, $1 { }

