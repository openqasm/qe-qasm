OPENQASM 3.0;

// 1. Declaration with initialization.
int za = -123;

// 2. += operator.
za += 4;

// 3. Declaration with initialization.
uint zb = 301;

// 4. -= operator.
zb -= 8;

// 5. *= operator.
za *= 2;

// 6. *= operator.
zb /= 3;

// 7. Declaration with initialization.
uint zc = 1741;

// 8. <<= operator.
zc <<= 3;

// 9. += and >>= operators.
zb += za >>= 1;

// 10. Logical OR.
zc = zb || za;

// 11. Logical AND.
zc += zb && za;

// 12. Bitwise OR.
zc -= zb | za;

// 13. Bitwise AND.
zc = zb & za;

// 14. XOR.
zc = zb ^ za;

// 15. Arithmetic negate.
zc = -zb;

// 16. Boolean negate.
zc = !(zb ^ za);

// 17. Boolean negate with different semantics.
zc = !zb ^ za;

// 17. More complicated boolean negate.
zc = !zb && !za;

// 18. Pre-increment identifier.
zb = ++zc;

// 19. Pre-increment expression.
za = (zb | zc)++;

// 20. Assignment chain.
za = zb = zc;

// 21. Self-Assignment chain.
za <<= zb ^= zc;

// 22. Unary Ops - sin.
za -= sin(zb);

// 23. Unary Ops - cos.
zc += cos(za);

// 24. Unary Ops - sqrt.
zb -= sqrt(zc);



