    .intel_syntax noprefix
    .global _start
    .text

.equ chunck_size, 32 * 1024 * 1024

.macro get_break
    mov rax, 12
    mov rdi, 0
    syscall
.endm

.macro allocate_chunk
    mov rax, [heap_end]
    lea rdi, [rax + chunck_size]
    mov rax, 12
    syscall
    add [heap_end], rax
.endm

_start:
    call read_input

    mov rax, [carret]
    cmp rax, [heap_start]
    je exit

// if last char '\n' - output
    dec rax
    movb bl, [rax]
    cmp bl, '\n'
    je output

// else - put it and increase carret
    inc rax
    movb [rax], '\n'
    inc rax
    mov [carret], rax

output:
//    mov rax, 1
//    mov rdi, 1
//    mov rsi, [heap_start]
//    mov rdx, [carret]
//    sub rdx, rsi
//    syscall

    call reverse_output

exit:
// exit(0) {
    mov rax, 60
    xor rdi, rdi
    syscall
// }

reverse_output:
    mov rdi, [carret]
    dec rdi

reverse_output_loop:
// count symbols before us
    mov rcx, rdi
    sub rcx, [heap_start]
    mov rdx, rcx

    cmp rcx, 0
    je reverse_output_return

// move one char before '\n'
    dec rdi

// find next '\n'
    std
    mov al, '\n'
    repne scasb

// if didn't find - it is first line
    jne reverse_output_return

// count length
    sub rdx, rcx

    inc rdi
    push rdi
    inc rdi

// write(stdout, rdi, rdx) {
    mov rax, 1
    mov rsi, rdi
    mov rdi, 1
    syscall
// }

    pop rdi

    jmp reverse_output_loop

reverse_output_return:
    sub rdx, rcx
    inc rdx
    mov rax, 1
    mov rdi, 1
    mov rsi, [heap_start]
    syscall

    ret

read_input:
    get_break
    mov [carret], rax
    mov [heap_start], rax
    mov [heap_end], rax
    allocate_chunk

read_loop:
// read(stdin, carret, heap_end - carret) {
    xor rax, rax
    xor rdi, rdi
    mov rsi, [carret]
    mov rdx, [heap_end]
    sub rdx, [carret]
    syscall
// }

    cmp rax, 0
// if (rax == 0) return;
    je read_return

// carret += rax
    add [carret], rax

    mov rax, [heap_end]
    cmp rax, [carret]

// if (heap_end - carret > 0) continue;
    jne read_loop

    allocate_chunk
    jmp read_loop

read_return:
    ret
    .data

carret:
.quad 0

heap_start:
.quad 0

heap_end:
.quad 0
