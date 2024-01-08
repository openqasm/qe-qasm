OPENQASM 3.0;

qubit $0;

defcal dcal(lambda, theta) $0 { }

float[64] x = 0.350350;

duration d = durationof( { dcal(pi / 2, sin(x)) $0; } );
