OPENQASM 3.0;

include "test.inc";

gate foo a {
  phase(-pi / 2) a;
}

gate bar q {
  phase(pi / 2) q;
}
