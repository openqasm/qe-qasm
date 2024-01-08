OPENQASM 3.0;

gate j q { }

gate k q { }

gate l q { }

gate m q { }

gate n q { }

gate a q { }

gate b q { }

gate c q { }

gate d q { }

gate e q { }


bit[5] x;
bit[5] y;

qubit $0;
qubit $1;
qubit $2;
qubit $3;
qubit $4;

if (x[0] == 0) {
  j $0;
  if (x[1] == 0) {
    k $1;
    if (x[2] == 0) {
      l $2;
      if (x[3] == 0) {
        m $3;
        if (x[4] == 0) {
          n $4;
        } else {
          e $4;
        }
      } else {
        d $3;
      }
    } else {
      c $2;
    }
  } else {
    b $1;
  }
} else {
  a $0;
  b $1;
  c $2;
  d $3;
  e $4;
}
