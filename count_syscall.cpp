#include <stdio.h>
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <assert.h>
#include <sys/user.h>
#include <stdlib.h>
#include <cstring>
#include <errno.h>

#define ERROR_EXIT(x, ...)    { ERROR_EXIT_##x(__VA_ARGS__) }
#define ERROR_EXIT_4(str1, ...)     fprintf(stderr, "%s ", str1);    ERROR_EXIT_3(__VA_ARGS__)
#define ERROR_EXIT_3(str2, ...)     fprintf(stderr, "%s ", str2);    ERROR_EXIT_2(__VA_ARGS__)
#define ERROR_EXIT_2(str3, ...)     fprintf(stderr, "%s ", str3);    ERROR_EXIT_1(__VA_ARGS__)
#define ERROR_EXIT_1(str4)              \
    fprintf(stderr, "%s:\t%s\n",        \
            str4, strerror(errno));     \
    fflush(stderr);                     \
    exit(EXIT_FAILURE);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s PROGRAM\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t child;
    if ((child = fork()) < 0)       ERROR_EXIT(1, "fork error")     // use fork to run tracee in the child process

    // tracee
    if (child == 0) {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)    ERROR_EXIT(1, "PTRACE_TRACEME error @chlid")
        if (execvp(argv[1], argv+1) < 0)            ERROR_EXIT(1, "execvp error")
    }

    //tracer
    else {
        int status;
        int syscall_enter = 0x01;                       // determine enter or exit syscall_stop
        int is_exit = 0;                                // determine whether this syscall is 'exit' or 'exit_group'
        struct user_regs_struct regs;                   // for storing registers
        unsigned long long int syscall_count = 0;       // for counting how many syscalls are invoked
        if (waitpid(child, &status, 0) < 0)     ERROR_EXIT(1, "waitpid error")
        assert(WIFSTOPPED(status));
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL | PTRACE_O_TRACESYSGOOD);
        
        while (WIFSTOPPED(status)) {
            if (ptrace(PTRACE_SYSCALL, child, 0, 0) < 0)    ERROR_EXIT(1, "PTRACE_SYSCALL error @parent")
            if (waitpid(child, &status, 0) < 0)             ERROR_EXIT(1, "waitpid error")
            if ( !is_exit) {
                if (ptrace(PTRACE_GETREGS, child, NULL, &regs) < 0)
                    ERROR_EXIT(1, "PTRACE_GETREGS error @parent") 
            }
            // syscal_stop
            if (WSTOPSIG(status) & 0x80) {
                if (syscall_enter) {        // enter to syscall
                    syscall_count++;
                    fprintf(stdout, "0x%llx: rax=%llx rdi=%llx rsi=%llx rdx=%llx r10=%llx r8=%llx r9=%llx\n",
                            regs.rip-2, regs.orig_rax, regs.rdi, regs.rsi, regs.rdx, regs.r10, regs.r8, regs.r9);
                            // machine code of syscall = 0x0f 0x05, so rip needs to sub 2
                    if (regs.orig_rax == 0x3c || regs.orig_rax == 0xe7) {       // exit || exit_group
                        fprintf(stdout, "\n");
                        is_exit = 1;
                    }
                }
                else {      // exit from syscall
                    fprintf(stdout, "0x%llx: ret = 0x%llx\n", regs.rip-2, regs.rax);
                }
                syscall_enter ^= 0x01;
            }
        }
        fprintf(stdout, "## %lld syscall(s) executed.\n", syscall_count);
    }

    return 0;
}
