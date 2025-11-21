syscall1:
	movq	%rdi, %rax
	movq	%rsi, %rdx
	syscall
	ret
	.globl	_start
_start:
	movq	$0, %rsi
	movq	$60, %rdi
	call	syscall1
