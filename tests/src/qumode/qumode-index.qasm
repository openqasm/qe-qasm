OPENQASM 3.0;

include "cvgates.inc";


qumode[2] qm;

disp(0.3 + 0.5 im) qm[0];
// snap([pi/2, 0, 0.3]) qm[1];

snap[3]([pi/2, 0, pi/3]) qm;
// qubit qb;


// ecd(0.3 + 0.5 im) qb[0], qm[0];
