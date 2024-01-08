OPENQASM 3.0;

int i = 15;
int j = 25;
int k = 35;

float f1 = pi;
float f2 = pi;
float f3 = pi;

while (i != 0) {
  f1 += i;
  --i;

  while (j != 0) {
    f2 += j;
    --j;

    while (k != 0) {
      f3 += k;
      --k;
    }
  }
}
