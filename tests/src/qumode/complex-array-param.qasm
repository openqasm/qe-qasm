OPENQASM 3.0;

include "cvgates.inc";

gate disp3(array[complex[float[64]], 3] alphas) qumode a, qumode b, qumode c {
  disp(alphas[0]) a;
  disp(alphas[1]) b;
  disp(alphas[2]) c;
}

qumode[3] qm;

disp3([0.3 + 0.5 im, 0.2 + 0.4 im, 0.1 + 0.3 im]) qm[0], qm[1], qm[2];
