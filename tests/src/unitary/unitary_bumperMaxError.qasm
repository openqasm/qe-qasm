OPENQASM 3.0;

unitary u;

u.bumper_max += 3; //should throw a syntax error, unexpected TOK_ADD_ASSIGN, expecting '=' 







unitary v;

v.bumper = 1
v.bumper += 2 //(must define bumper first)


r.bumper_max +=3 //never works