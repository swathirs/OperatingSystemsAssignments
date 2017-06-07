.global thread_switch

thread_switch:
pushq %rbx
push %rbp
push %r12
push %r13
push %r14
push %r15
movq %rsp, (%rdi)
movq (%rsi), %rsp
popq %r15
popq %r14
popq %r13
popq %r12
popq %rbp
popq %rbx
ret

