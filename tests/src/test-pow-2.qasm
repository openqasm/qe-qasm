OPENQASM 3.0;

complex[float[32]] cf0 = 7.95 * 1 im;

complex[float[32]] cf1 = 2.95 + 1 im;

float[32] f0 = 6.0;

float[32] f1 = 3.0;

float[64] f2 = f0 ** f1;

complex[float[64]] rf = cf0 ** cf1;
