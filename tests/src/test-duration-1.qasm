OPENQASM 3.0;

// controlled-NOT Gate
gate cx c, t {
  CX c, t;
}

// Clifford gate: Hadamard
gate h a {
  U(pi / 2, 0, pi) a;
}

// 1. Fixed duration, in standard units
duration a = 300ns;

// 2. Fixed duration, backend dependent
duration b = 800dt;

// 3. Fixed duration, referencing the duration of a calibrated gate
duration c = durationof(h);

// 4. Fixed duration, referencing the duration of a calibrated gate
qubit $0;

duration hxc = durationof( { h $0; } );

// 5. Dynamic duration, referencing a box within its context
qubit[8] q;

box bx {
  cx q[0], q[1];
  reset q;
}

duration d = durationof(bx);

duration la = 500ms;

duration xa = 350μs;

