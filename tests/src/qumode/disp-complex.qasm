OPENQASM 3.0;

qumode qm;
qubit qb;

disp(3+4im) qm;
disp(0.3 + 0.5 im) qm;
disp(0.3) qm;

complex[float[64]] c = 0.3 + 0.5 im;
disp(c) qm;
