OPENQASM 3.0;

int i = 3;
int j = 4;

if (i == j) {
  cal {
    extern port p0;
    frame driveframe1 = newframe(p0, 5.1e9, 0.0);
  }
}

