OPENQASM 3.0;

gate dgate(lambda, theta) q {
  U(lambda, pi, theta) q;
}

qubit $0;

duration a = 300ns;

duration b = durationof({ dgate(pi / 2, -pi / 2) $0; });

stretch c;

// stretchy duration with min=300ns
duration d = a + 2 * c;

// stretchy duration with backtracking by up to half b
duration e = -0.5 * b + c;
