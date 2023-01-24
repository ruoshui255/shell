#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "parse.h"

#define length(x) (sizeof(x)/sizeof(x[0]))

struct cmd* cmd_parse(char* buf);



void
diff_ecmd(struct cmd* cmd, char** expected, int argc){
    assert(cmd->type == cmdtype_exec);
    
    struct cmd_exec* ecmd = (struct cmd_exec*)cmd;
    assert(ecmd->argc == argc);

    for (int i = 0; i < ecmd->argc; i ++) {
        if (! equal_string(ecmd->argv[i], expected[i])){
            print_diff_exit(cmd, expected, argc);
        }
    }
}

void 
test1() {
    char buf[20] = "ls|grep 123" ;

    char* expected1[] = {"ls"};
    char* expected2[] = {"grep", "123"};
    
    struct cmd* cmd = cmd_parse(buf);
    assert(cmd->type == cmdtype_pipe);
   
    struct cmd_pipe* pcmd = (struct cmd_pipe*) cmd;
    diff_ecmd(pcmd->left, expected1, length(expected1));
    diff_ecmd(pcmd->right, expected2, length(expected2));
    
    return;
}


void 
test2() {
    char buf[20] = "ls|grep \"123\"" ;

    char* expected1[] = {"ls"};
    char* expected2[] = {"grep", "123"};
    
    struct cmd* cmd = cmd_parse(buf);
    assert(cmd->type == cmdtype_pipe);
   
    struct cmd_pipe* pcmd = (struct cmd_pipe*) cmd;
    diff_ecmd(pcmd->left, expected1, length(expected1));
    diff_ecmd(pcmd->right, expected2, length(expected2));
    
    return;
}

int 
main(int argc, char const *argv[]) {
    test2();    
    return 0;
}