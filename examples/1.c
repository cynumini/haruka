unsigned long int syscall1(unsigned long int number, unsigned long int arg1) {
  unsigned long int ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(number), "D"(arg1)
               : "rcx", "r11", "memory");
  return ret;
}
void _start() { syscall1(60, 10); }
