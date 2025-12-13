unsigned long int syscall1(unsigned long int number, unsigned long int arg1) {
  unsigned long int ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(number), "D"(arg1)
               : "rcx", "r11", "memory");
  return ret;
}

unsigned long int syscall3(unsigned long int number, unsigned long int arg1,
                           unsigned long int arg2, unsigned long int arg3) {
  unsigned long int ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(number), "D"(arg1), "S"(arg2), "d"(arg3)
               : "rcx", "r11", "memory");
  return ret;
}

void _start() {
  syscall3(1, 0, (unsigned long int)"nya\n", 4);
  syscall1(60, 0);
}
