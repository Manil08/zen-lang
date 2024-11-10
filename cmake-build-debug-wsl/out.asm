global _start
fibonacci:
    push QWORD [rsp + 8]
    mov rax, 1
    push rax
    pop rax
    pop rbx
    cmp rax, rbx
    jne S11
    push 1
    jmp S21
S11:
    push 0
S21:
    pop rax
    cmp rax, 1
    jne else1
    mov rax, 1
    push rax
    pop rax
    ret
    jmp end1
else1:
end1:
    push QWORD [rsp + 8]
    mov rax, 2
    push rax
    pop rax
    pop rbx
    cmp rax, rbx
    jne S13
    push 1
    jmp S23
S13:
    push 0
S23:
    pop rax
    cmp rax, 1
    jne else3
    mov rax, 1
    push rax
    pop rax
    ret
    jmp end3
else3:
end3:
    push QWORD [rsp + 8]
    mov rax, 1
    push rax
    pop rax
    pop rbx
    sub rbx, rax
    push rbx
    call fibonacci
    pop rbx
    push rax
    push QWORD [rsp + 16]
    mov rax, 2
    push rax
    pop rax
    pop rbx
    sub rbx, rax
    push rbx
    call fibonacci
    pop rbx
    push rax
    pop rax
    pop rbx
    add rax, rbx
    push rax
    pop rax
    ret
_start:
    mov rax, 3
    push rax
    call fibonacci
    pop rbx
    push rax
    pop rdi
    mov rax, 60
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
