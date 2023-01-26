OPENQASM 3.0;

int i = 3;

uint u = 7U;

int[128] xi = 310510710;

uint[128] xu = 310510710U;

float f0 = 12345.54321;

float f1 = 54321.12345;

float[128] xf0 = 1234512345.5432154321;

float[128] xf1 = 5432154321.1234512345;

float[128] x0 = pow(i, 2);

float[128] x1 = pow(u, 8);

float[128] x2 = pow(xi, i);

float[128] x3 = pow(xu, u);

float[128] x4 = pow(xu, u);

float[128] x5 = pow(f0, i);

float[128] x6 = pow(f1, u);

float[128] x7 = pow(xf0, f0);

float[128] x8 = pow(xf1, f1);

float[128] x9 = pow(xf0, xi);

float[128] x10 = pow(xf1, xu);

float[128] x11 = pow(xf0, cos(f0));

float[128] x12 = pow(xf0, sin(xf1));

float[128] x13 = pow(xf0 * 2, xf1 / 2);

float[128] x14 = pow(xi, xf1);

float[128] x15 = pow(xf0, xu);

