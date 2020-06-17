# Trace Me

Use GDB


## Steps:

1. Set break point at line 42.

2. Set two break points at the end of program (i.e. line 48).

```bash
    (gdb) b 42 48
    Breakpoint 1 at 0x853: file traceme.c, line 42.
    Breakpoint 2 at 0x8bc: file traceme.c, line 48.
```

3. Start the program.

```bash
    (gdb) r
    Starting program: /home/stan/nctu/unix_programing/projects/ptrace_practice/supplements/traceme2
```

4. The first break point:

```bash
    Breakpoint 1, main (argc=1, argv=0x7fffffffda08) at traceme.c:42
    42      traceme.c: No such file or directory.
    (gdb) c
    Continuing.
```

5. After continue:

```bash
    Program received signal SIGTRAP, Trace/breakpoint trap.
    __GI_raise (sig=<optimized out>) at ../sysdeps/unix/sysv/linux/raise.c:51
    51      ../sysdeps/unix/sysv/linux/raise.c: No such file or directory.
    (gdb) c
    Continuing.
    traced
```

6. At the end of the program,  

    Got the flag: "ASM{a_Pr0ce55_can_b_trac3d_0n1Y_0nc3}"

```bash
    Breakpoint 2, main (argc=1, argv=0x7fffffffda08) at traceme.c:48
    48      traceme.c: No such file or directory.
    (gdb) display output
    1: output = "ASM{a_Pr0ce55_can_b_trac3d_0n1Y_0nc3}", '\000' <repeats 26 times>
```
