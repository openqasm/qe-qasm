OPENQASM 3.0;

int i;

gate x q { }

gate y q { }

qubit $0;
qubit $1;
qubit $2;
qubit $3;
qubit $4;
qubit $5;
qubit $6;
qubit $7;

if (i == 1)
  x $0;
else if (i == 2)
  x $1;
else if (i == 3)
  x $2;
else if (i == 4)
  x $3;
else if (i == 5)
  x $4;
else if (i == 6)
  x $5;
else if (i == 7)
  x $6;
else if (i == 8)
  x $7;
else
  y $1;

