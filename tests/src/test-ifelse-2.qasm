OPENQASM 3.0;

angle k = 12345.6789;
angle j = 54321.9876;

float a = 31.3;
float b = 62.6;

float[128] f0;
float[128] f1 = pi;

// 1. Simple one-line expression test.
if (k == a) {
  f0 = cos(f1);
} else if (j == b) {
  f0 = sin(f1) + k;
}
