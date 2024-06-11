OPENQASM 3.0;


gate phase(lambda) q {
  U(0, 0, lambda) q;
}

qubit a;

array[qubit, 16] aarray;

input array[angle, 16] aa;


phase(aa[0]) a;

phase(aa[0]) aarray;

angle assign = aa[0];

phase(assign) a;
