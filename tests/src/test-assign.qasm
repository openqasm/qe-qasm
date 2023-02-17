OPENQASM 3.0;

angle[3] c = 0;

// Assign.
c = 1;

// Another Assign.
uint j = 14;

// Transitive Assign
uint k = c = j;

// Transitive Self-Assign
uint m = c *= j;

// Assign from Pre/Post.
uint n = m + j++;

// Assign from Add + Self-Assign
uint p = n + m << j;

