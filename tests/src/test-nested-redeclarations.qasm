OPENQASM 3.0;

int i = 1;

int j = 10;

int k = 20;

if (j > i) {
  int i = 5;
  int k = 25;

  if (k > i) {
    int i = 10;
    int k = 35;

    if (k > i) {
      int i = 15;
      int k = 45;
    } else {
      int i = -15;
      int k = -45;
    }
  } else {
    int i = -10;
    int k = -35;
  }
}

