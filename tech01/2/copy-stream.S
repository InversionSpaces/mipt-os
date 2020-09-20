    .intel_syntax noprefix
    .global _start
    .text
.equ buffer_size, 1024
_start:
// rax = read(stdin, buffer, buffer_size) {
    mov rax, 0
    mov rdi, 0
    mov rsi, OFFSET buffer
    mov rdx, buffer_size

    syscall
// }

    cmp rax, 0
    jbe end

// write(stdout, buffer, rax)
    mov rdx, rax
    mov rax, 1
    mov rdi, 1
    mov rsi, OFFSET buffer

    syscall
// }

    jmp _start

end:
// exit(rax) {
    mov rdi, rax
    mov rax, 60

    syscall
// }

    .data
buffer:
    .skip buffer_size
