// CV / qumode gate definitions.
// disp is a builtin gate with the following signature:
// gate disp(complex[float[64]] alpha) qumode qm;

gate snap[uint N](array[angle, N] thetas) qumode qm {
    for i in [0:N] {
        ctrl[i] @ gphase(thetas[i]) qm;
    }
}

gate ecd(complex[float[64]] alpha) qubit qb, qumode qm {
    negctrl @ disp(-alpha/2) qb, qm;
    ctrl @ disp(alpha/2) qb, qm;
}
