/*
&A[i][j] in %rdx
&A[j][i] in %rax
addq $8, %rdx # means A[i][j] -> A[i][j+1]
addq $120, %rax # means A[j][i] -> A[j+1][i], 120 == 8*M
*/