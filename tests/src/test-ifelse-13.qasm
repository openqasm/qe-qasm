OPENQASM 3.0;

int i = 15;
int j = 30;
int k = 45;

if (i < 30 && j > 25) {
  uint k = 30 * 2;
  k -= j;
} else {
  uint k = 30 / 2;
  k += i;
}

