#!/usr/bin/python3

import re

rgx = re.compile(r'(?<=SYS_)\w+')

syscalls = []

with open('syscall.h', 'r') as file:
    for ln, line in enumerate(file.readlines()):
        result = rgx.findall(line)

        if len(result) == 1:
            syscalls.append(result[0])

with open('syscall_names.c', 'w') as file:
    file.write(
      '#include "syscall.h"\n\n'
      + 'char *syscall_names[] = {\n'
      + ',\n'.join([f'[SYS_{syscall}]\t"{syscall}"' for syscall in syscalls])
      + '};'
    )
