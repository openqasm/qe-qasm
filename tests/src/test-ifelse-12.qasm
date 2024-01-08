OPENQASM 3.0;

// 3-parameter 2-pulse single qubit gate
gate u3(theta,phi,lambda) q { U(theta,phi,lambda) q; }

// 2-parameter 1-pulse single qubit gate
gate u2(phi,lambda) q { U(pi/2,phi,lambda) q; }

// 1-parameter 0-pulse single qubit gate
gate u1(lambda) q { U(0,0,lambda) q; }

int i = 35;
int j = 40;
int k = 45;

// 1. Dangling else-if (no closing else) expression test.
if (i == j || i > k) {
  u1(-pi / 4) a;
  u1(-pi / 4) a;
  u1(-pi / 4) a;
} else {
  if (k > j && j > i) {
    u2(-pi / 2, 0) b;
    u2(-pi / 2, 0) b;
    u2(-pi / 2, 0) b;
  } else {
    u3(pi, 0, pi) c;
    u3(pi, 0, pi) c;
    u3(pi, 0, pi) c;
  }
}
