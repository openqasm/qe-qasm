OPENQASM 3.0;

def foo(int[16]:a) b -> bit {
}

int x = 1;
int y = 2;

foo(x + y % 5);

def bar(float[32]:a, qubit:q) j -> bit {
}

float j = 3.5;

bar(j % 3, %0);

