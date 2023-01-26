OPENQASM 3.0;

int bs = 0;

int i = 10;

int j = 20;

do {
  bs |= i--;

  while (j != 0) {
    bs = bs + 1;
    --j;
  }

} while (i > 3);

