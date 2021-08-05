/*
.pos 0
	irmovq stack, %rsp
	call main
	halt

# Array of 4 elements
.align 8
array:
  .quad 0x0000000000000000
  .quad 0x0000000000000000
  .quad 0x0000000000000000
  .quad 0x0000000000000000

main:
  # test number 1, -1, 3, 5
  irmovq array, %r10

  irmovq $1,%rdi
	call switchv
  rmmovq %rax, (%r10)

  irmovq $-1,%rdi
	call switchv
  rmmovq %rax, 8(%r10)

  irmovq $3,%rdi
	call switchv
  rmmovq %rax, 16(%r10)

  irmovq $5,%rdi
	call switchv
  rmmovq %rax, 24(%r10)
	ret

table:
  .quad LD  # default branch
  .quad L0  # idx == 0
  .quad L1  # idx == 1
  .quad L2  # idx == 2
  .quad L3  # idx == 3
  .quad L4  # idx == 4
  .quad L5  # idx == 5

# long switchv(long idx)
# idx in %rdi
switchv:
  # contant number
  irmovq $8, %r8
  irmovq $0, %r10
  irmovq $1, %r11

  irmovq $0, %rax
  irmovq table, %rcx # table address
  rrmovq %rdi, %rdx
  subq %r8, %rdx
  jg def 			 # idx > 5
  subq %r10, %rdi
  jl def 			 # idx < 0
mul: # calculate 8 * %rdi
  subq %r10, %rdi
  je addr
  addq %r8, %rcx
  subq %r11, %rdi
  jmp mul
addr: # jump using table address
  addq %r8, %rcx
  mrmovq (%rcx), %rdi
  pushq %rdi
  ret
def: # default branch
  irmovq table, %rcx
  mrmovq (%rcx), %rdi
  pushq %rdi
  ret
L0:
  irmovq $0xaaa, %rax
  ret
L1:
  jmp LD
L2:
  jmp L5
L3:
  irmovq $0xccc, %rax
  ret
L4:
  jmp LD
L5:
  irmovq $0xbbb, %rax
  ret
LD:
  irmovq $0xddd, %rax
  ret

.pos 0x200
stack:
*/