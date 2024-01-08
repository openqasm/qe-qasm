OPENQASM 3.0;

qubit[16] q;

qubit[3] z;

let j = q[4] || q[8] || z[0];

let k = q[7] ++ q[13] ++ z[1];

let n = q[3] || q[6] || q[9] || z;

let x = q[3] ++ q[6] ++ z ++ q[9];
