OPENQASM 3.0;

qubit $0;
qubit $1;

float[64] f = 0.543212345;

defcal rx(pi / 2, atan(f)) $0 { }

defcal ry(cos(f), -pi / 2) $1 { }

