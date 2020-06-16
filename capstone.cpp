#include <stdio.h>
#include <csignal>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <iostream>
#include <capstone/capstone.h>

using namespace std;

void handler(int signum) {
    cout << " Caught interupt, exit." << endl;
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    char machine_code[4096];
    char input_str[4096];
    signal(SIGINT, handler);
    
    while (1) {
        memset(machine_code, 0, sizeof(machine_code)/sizeof(char));
        memset(input_str, 0, sizeof(machine_code)/sizeof(char));

        cin.getline(input_str, sizeof(input_str));          // e.g. input: '>>> 4839f048ffcb4889db4885fa4801d3'
        if (strlen(input_str) == 0)     continue;
        strcpy(machine_code, input_str);                  // remove first 4 charactors (i.e. '>>> ')
        
        int size = strlen(machine_code)/2;
        unsigned char code_hex[size + 1];

        // Convert string to heximal value one byte each.
        for (int i=0 ; i<size ; i++) {
            char tmp[3];
            strncpy(tmp, machine_code + (i*2), sizeof(tmp)-1);
            tmp[2] = '\0';
            code_hex[i] = (unsigned char)strtol(tmp, NULL, 16);
        }
        code_hex[size] = '\0';

        // for (int i=0 ; i<size ; i++) {
        //     printf("%d: %x\n", i, code_hex[i]);
        // }

        csh cshandle = 0;
        cs_insn *insn;
        size_t count;
        char readable_assmably[4096];
        unsigned char machine_bytes[9192];
        int mbsize;
        if (cs_open(CS_ARCH_X86, CS_MODE_64, &cshandle) != CS_ERR_OK)       return -1;
        if ((count = cs_disasm(cshandle, (unsigned char*)code_hex, sizeof(code_hex), 0x1000, 0, &insn)) > 0) {
            for (size_t j=0 ; j<count ; j++) {
                memset(readable_assmably, 0, 4096);
                sprintf(readable_assmably, "%s  %s\n", insn[j].mnemonic, insn[j].op_str);
                mbsize = insn[j].size;
                memcpy(machine_bytes, insn[j].bytes, insn[j].size);
                // // Output assambly in hex.
                // for (size_t i=0 ; i<strlen(readable_assmably) ; i++) {
                //     cout << setw(2) << setfill('0') << hex << int(readable_assmably[i]);
                // }
                // Output machine code in hex.
                for (int i=0 ; i<mbsize ; i++) {
                    cout << setw(2) << setfill('0') << hex << int(machine_bytes[i]) << " ";
                }
                // Output assambly in human-readable string.
                cout << "\t\t" << readable_assmably;
            }
            cout << endl;
            cs_free(insn, count);
        }
        else {
            fprintf(stderr, "disassamble error");
            exit(EXIT_FAILURE);
        }
        cs_close(&cshandle);
    }
}