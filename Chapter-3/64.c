/* &D[i][j][k] = Xd + L(S*T*i + T*j + k)
store_ele:
  leaq (%rsi, %rsi, 2), %rax    # t1 = j*3
  leaq (%rsi, %rax, 4), %rax    # t1 = j*13
  movq %rdi, %rsi               # t2 = i
  salq $6, %rsi                 # t2 = i*64
  addq %rsi, %rdi               # t3 = i*65
  addq %rax, %rdi               # t3 = i*65 + j*13
  addq %rdi, %rdx               # t4 = i*65 + j*13 + k
  movq A(,%rdx,8), %rax         # t1 = *(A + 8*t4)
  movq %rax, (%rcx)             # *dest = t1
  movl $3640, %eax              # return 3640
  ret
*/