OPENQASM 3.0;

int i = 15;
int j = 25;
int k = 35;

float f1 = pi;
float f2 = pi;
float f3 = pi;

for x in [0 : 100] {
  f1 += i;
  i += x;

  for y in [0 : 100] {
    f2 += j;
    j += y;

    for z in [0 : 100] {
      f3 += k;
      k += z;
    }
  }
}

