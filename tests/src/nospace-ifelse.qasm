OPENQASM 3.0;

include "stdgates.inc";

uint k = 3;
uint l = 5;
uint m = 7;

float[128] f0;
float[128] f1;

// Testing conditionals without space.
if(m == k) {
  f0 = pi / 2;
} else {
  f0 = -pi / 2;
}

// Testing conditionals with spurious newlines.
if(l == k) {
  f1 = sin(f0);
} else
  if(k != m) {
    f1 = sin(f0) / cos(f0);
} else {
  f1 = atan(f0) / asin(f0);
}
