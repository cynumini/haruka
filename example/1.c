typedef unsigned long u64;

u64 syscall1(u64 number, u64 arg1) {
  u64 ret;
  asm("syscall" : "=a"(ret) : "a"(number), "d"(arg1) : "rcx", "r11", "memory");
  return ret;
}

void _start() { syscall1(60, 0); }
