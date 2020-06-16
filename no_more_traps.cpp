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
#include <string>

#define OP_CODE_FILE    "./supplements/no_more_traps.txt"

#define ERROR_EXIT(x, ...)          { ERROR_EXIT_##x(__VA_ARGS__) }
#define ERROR_EXIT_4(str1, ...)     fprintf(stderr, "%s ", str1);    ERROR_EXIT_3(__VA_ARGS__)
#define ERROR_EXIT_3(str2, ...)     fprintf(stderr, "%s ", str2);    ERROR_EXIT_2(__VA_ARGS__)
#define ERROR_EXIT_2(str3, ...)     fprintf(stderr, "%s ", str3);    ERROR_EXIT_1(__VA_ARGS__)
#define ERROR_EXIT_1(str4)              \
    fprintf(stderr, "%s:\t%s\n",        \
            str4, strerror(errno));     \
    fflush(stderr);                     \
    exit(EXIT_FAILURE);

void dump_code(long addr, long code) {
	fprintf(stderr, "0x%lx: code = %02x %02x %02x %02x %02x %02x %02x %02x\n",
		addr,
		((unsigned char *) (&code))[0],
		((unsigned char *) (&code))[1],
		((unsigned char *) (&code))[2],
		((unsigned char *) (&code))[3],
		((unsigned char *) (&code))[4],
		((unsigned char *) (&code))[5],
		((unsigned char *) (&code))[6],
		((unsigned char *) (&code))[7]);
}

unsigned long get_next_opcode(FILE *fp) {
    char byte_str[3];
    memset(byte_str, 0, sizeof(byte_str));
    for (int i=0 ; i<2 ; i++)
        byte_str[i] = (char)fgetc(fp);
    // printf("%s %.2lx\n", byte_str, patch_code);
    return strtoul(byte_str, NULL, 16);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s PROGRAM\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fp;
    if ((fp = fopen(OP_CODE_FILE, "r")) == NULL) ERROR_EXIT(1, "fopen error")

    pid_t child;
    struct user_regs_struct regs;                   // for storing registers
    unsigned long code, target, patched;
    if ((child = fork()) < 0)       ERROR_EXIT(1, "fork error")     // use fork to run tracee in the child process

    // tracee
    if (child == 0) {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)    ERROR_EXIT(1, "PTRACE_TRACEME error @chlid")
        if (execvp(argv[1], argv+1) < 0)            ERROR_EXIT(1, "execvp error")
    }

    //tracer
    else {
        int status;
        if (waitpid(child, &status, 0) < 0)     ERROR_EXIT(1, "waitpid error")
        assert(WIFSTOPPED(status));
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL);
        if (ptrace(PTRACE_CONT, child, 0, 0) < 0)            ERROR_EXIT(1, "PTRACE_CONT error @parent")

        while (waitpid(child, &status, 0)) {
            // encounter 0xcc will generate SIGTRAP
            if (WIFSTOPPED(status)) {
                if (ptrace(PTRACE_GETREGS, child, NULL, &regs) < 0)     ERROR_EXIT(1, "PTRACE_GETREGS error @parent")
                target = regs.rip - 1;
                if ((code = ptrace(PTRACE_PEEKTEXT, child, target, 0)) < 0)  ERROR_EXIT(1, "PTRACE_PEEKTEXT error @parent")

                // patch new opcode
                if ((code & 0x00000000000000ff) == 0xcc) {
                    // dump_code(target, code);
                    patched = (code & 0xffffffffffffff00) | get_next_opcode(fp);
                    // dump_code(target, patched);

                    if(ptrace(PTRACE_POKETEXT, child, target, patched) < 0)      ERROR_EXIT(1, "PTRACE_POKETEXT error @parent")
                    // set regs
                    regs.rip = regs.rip - 1;
                    if(ptrace(PTRACE_SETREGS, child, 0, &regs) < 0) ERROR_EXIT(1, "PTRACE_SETREGS error @parent")
                }
                else {
                    fprintf(stderr, "Unknown SIGSTOP @0x%llx", regs.rip);
                    // exit(EXIT_FAILURE);
                    sleep(5);
                }
            }
            if (WIFEXITED(status)) {
                fprintf(stderr, "Child exit state: %d\n", WEXITSTATUS(status));
                break;
            }
            if (ptrace(PTRACE_CONT, child, 0, 0) < 0)   ERROR_EXIT(1, "PTRACE_CONT error @parent")
        }
    }

    fclose(fp);
    return 0;
}