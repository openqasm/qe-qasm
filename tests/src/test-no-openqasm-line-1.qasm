int i = 3;

qubit $0;
qubit $1;

gate foo q { }

switch (i) {
  case 0:
    foo $0;
  break;
  case 1:
    foo $1;
  break;
  default:
  break;
}


