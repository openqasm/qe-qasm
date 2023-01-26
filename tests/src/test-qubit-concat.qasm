OPENQASM 3.0;

qubit[16] q;

let j = q[4] || q[8];

let k = q[7] ++ q[13];

let n = q[3] || q[6] || q[9] || q[12];

let t = q[3] ++ q[5] ++ q[7] ++ q[11];

