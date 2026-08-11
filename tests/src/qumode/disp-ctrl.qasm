OPENQASM 3.0;

qubit qb;
qubit q0;
qubit q1;
qumode qm;

ctrl @ disp(0.5 + 0.3im) qb, qm;
ctrl(2) @ disp(0.3) q0, q1, qm;
