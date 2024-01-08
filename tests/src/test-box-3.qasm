OPENQASM 3.0;

include "stdgates.inc";

qubit[8] q;

boxto 1ms {
  stretch a;
  delay[a] q;

  cx q[0], q[1];
  delay[a] q[0];
}
