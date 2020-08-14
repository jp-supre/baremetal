.global port_in
.global port_out
.global get_tmr
port_in:
    mov %di, %dx
    in %dx, %eax
    ret
port_out:
    mov %di, %dx
    mov %esi, %eax 
    out %eax, %dx
    ret
get_tmr:
    mov %di, %dx
    in %dx, %eax
    ret