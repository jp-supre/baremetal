.global get_tmr
get_tmr:
  mov %di, %dx
  in %dx, %eax
  ret