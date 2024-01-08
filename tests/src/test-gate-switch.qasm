OPENQASM 3.0;

gate x q { }

qubit $0;

int i = 0;

switch (i) {
case 0: {
    x $0;
}
break;
default:
break;
}

x $0;
