OPENQASM 3.0;

int bs = 0;

int i = 10;

do {
  bs |= i--;
} while (i > 3);

