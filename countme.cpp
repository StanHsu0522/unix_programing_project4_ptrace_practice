#include <iostream>
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <assert.h>

#define ERROR_EXIT(str)         \
    {                           \
        cout << str << endl;    \
        exit(EXIT_FAILURE);     \
    }

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2)       ERROR_EXIT("Usage: traceme program.")


    pid_t child;
    if ((child = fork()) < 0)       ERROR_EXIT("fork error")
    if (child == 0) {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)    ERROR_EXIT("ptrace error @chlid")
        if (execvp(argv[1], argv+1) < 0)            ERROR_EXIT("execvp error")
    }
    else {
        int status;
        unsigned long long int ins_count = 0;
        if (waitpid(child, &status, 0) < 0)     ERROR_EXIT("waitpid error")
        assert(WIFSTOPPED(status));
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL);
        
        // Style-1
        while (WIFSTOPPED(status)) {
            ins_count++;
            if (ptrace(PTRACE_SINGLESTEP, child, 0, 0) < 0)     ERROR_EXIT("ptrace error @parent")
            if (waitpid(child, &status, 0) < 0)                 ERROR_EXIT("waitpid error")
        }
        if (WIFEXITED(status)) {
            cout << "child exit status: " << WEXITSTATUS(status) << endl;
            cout << "Total " << ins_count << " instruction(s) executed." << endl;
        }

        // // Style-2
        // ins_count++;
        // if (ptrace(PTRACE_SINGLESTEP, child, 0, 0) < 0)     ERROR_EXIT("ptrace error @parent")
        // while (waitpid(child, &status, 0)) {
        //     if (WIFEXITED(status)) {
        //         cout << "child exit status: " << WEXITSTATUS(status) << endl;
        //         cout << "Total " << ins_count << " instruction(s) executed." << endl;
        //         exit(EXIT_SUCCESS);
        //     }
        //     else if (WIFSTOPPED(status)) {
        //         ins_count++;
        //         if (ptrace(PTRACE_SINGLESTEP, child, 0, 0) < 0)     ERROR_EXIT("ptrace error @parent")
        //     }
        // }
    }
}