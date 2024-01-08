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
  if (y[0] == 0) {
    a $0;
    if (y[1] == 0) {
      b $1;
      if (y[2] == 0) {
        c $2;
        if (y[3] == 0) {
          d $3;
          if (y[4] == 0) {
            e $4;
          } else {
            n $4;
          }
        } else {
          m $3;
        }
      } else {
        l $2;
      }
    } else {
      k $1;
    }
  } else {
    j $0;
  }
}
