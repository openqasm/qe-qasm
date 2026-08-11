OPENQASM 3.0;
// unitary u;

// unitary v = {
//     {1, 0},
//     {0, 1}
// };

// qumode q;
// qubit qb;

// u q;
// u qb;

// v q;
// v qb;



qumode q;
qubit qb1;
qubit qb2;

snap([pi/2, 0]) q;
disp(0.5) q;
u q;

x qb1;
z qb1;
cx qb1, qb2;
u qb1;