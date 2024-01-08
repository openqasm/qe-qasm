OPENQASM 3.0;

gate dgate(lambda, theta) q { }

qubit $0;

float[64] x = 0.350350;

duration d = durationof( { dgate(pi / 2, sin(x)) $0; } );
