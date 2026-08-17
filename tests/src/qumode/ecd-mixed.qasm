OPENQASM 3.0;

include "cvgates.inc";

qubit qb;
qumode qm;

ecd(0.5) qb, qm;
snap([pi/2, 0, pi/3]) qm;
