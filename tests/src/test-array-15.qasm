OPENQASM 3.0;

float[128] fl = 53.35;

int[128] ii = 41;

uint[128] ui = 51;

array[complex[float[128]], 64] acf;

array[complex[float[128]], 64] aci;

array[complex[float[128]], 64] acu;

acf[0] = fl;

aci[0] = ii;

acu[0] = ui;

complex[float[128]] cf0 = acf[8];

complex[float[64]] cf1 = acf[16];

