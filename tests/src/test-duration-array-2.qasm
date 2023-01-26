OPENQASM 3.0;

gate testgate(theta) q {
  U(pi, theta, -pi) q;
}

qubit $0;

angle x = 0.50505;

array[durationof({ testgate(x) $0; }), 16] xdg;

