OPENQASM 3.0;

include "cvgates.inc";

qumode qm;

snap([pi/2, 0, pi/3]) qm;
snap([pi/2, 0, pi/3, pi/4]) qm;
snap([pi/2]) qm;
