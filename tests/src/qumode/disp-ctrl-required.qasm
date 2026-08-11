OPENQASM 3.0;

qubit qb;
qumode qm;

// Negative: controlled operand list requires ctrl/negctrl.
disp(0.5 + 0.3im) qb, qm;
