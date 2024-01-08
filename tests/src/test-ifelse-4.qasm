OPENQASM 3.0;

// Test gate expressions - no meaning.
gate tg2(lambda, theta) q, r  { }

gate tg3(lambda, theta, phi) q { }

angle e;
angle f;
angle g;

float[64] a;
float[64] b;

qubit $0;
qubit $1;

// 1. Dangling else-if (no closing else) expression test.
if (e == a) {
  tg3(pi, 0, pi) z;
  tg3(-pi, 0, -pi) $1;
} else {
  if (f == b) {
    tg2(-pi / 2, 0) $0, $1;
    tg2(-pi / 2, 0) $0, $1;
    tg2(-pi / 2, 0) $0, $1;
  } else {
    tg3(pi, 0, pi) $1;
    tg3(pi, 0, pi) $1;
    tg3(pi, 0, pi) $1;
  }
}
