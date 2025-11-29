.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp    # allocate locals
    movq $10, %rax
    movq %rax, -32(%rbp)
    movq $20, %rax
    movq %rax, -24(%rbp)
    movq -32(%rbp), %rax
    pushq %rax
    movq -24(%rbp), %rax
    movq %rax, %rbx
    popq %rax
    addq %rbx, %rax
    movq %rax, -32(%rbp)
    movq %rbp, %rsp
    popq %rbp
    ret
