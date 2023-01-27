#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "parse.h"

#define length(x) (sizeof(x)/sizeof(x[0]))

struct Cmd* cmdParse(char* buf);


void
diff_ecmd(struct Cmd* cmd, char** expected, int argc){
    assert(cmd->type == CmdTypeExec);
    
    struct CmdExec* ecmd = (struct CmdExec*)cmd;
    
    if (ecmd->argc != argc) {
        printf("cmd argc <%d> exp argc <%d>\n", ecmd->argc, argc);
        exit(-1);
    }

    for (int i = 0; i < ecmd->argc; i ++) {
        if (! equal_string(ecmd->argv[i], expected[i])){
            print_diff_exit(cmd, expected, argc);
        }
    }
}

void 
test1() {
    char buf[] = "ls|grep 123" ;

    char* expected1[] = {"ls"};
    char* expected2[] = {"grep", "123"};
    
    struct Cmd* cmd = cmdParse(buf);
    assert(cmd->type == CmdTypePipe);
   
    struct CmdPipe* pcmd = (struct CmdPipe*) cmd;
    diff_ecmd(pcmd->left, expected1, length(expected1));
    diff_ecmd(pcmd->right, expected2, length(expected2));
    
    printf("%s success\n", __FUNCTION__);
    return;
}


void 
test2() {
    char buf[] = "ls|grep \"123\"" ;

    char* expected1[] = {"ls"};
    char* expected2[] = {"grep", "123"};
    
    struct Cmd* cmd = cmdParse(buf);
    assert(cmd->type == CmdTypePipe);
   
    struct CmdPipe* pcmd = (struct CmdPipe*) cmd;
    diff_ecmd(pcmd->left, expected1, length(expected1));
    diff_ecmd(pcmd->right, expected2, length(expected2));
    
    printf("%s success\n", __FUNCTION__);
    return;
}

void 
test3() {
    char buf[] = "ls . ; grep 123" ;

    char* expected1[] = {"ls", "."};
    char* expected2[] = {"grep", "123"};
    
    struct Cmd* cmd = cmdParse(buf);
    assert(cmd->type == CmdTypeList);
   
    struct CmdList* lcmd = (struct CmdList*) cmd;
    diff_ecmd(lcmd->left, expected1, length(expected1));
    // diff_ecmd(lcmd->right, expected2, length(expected2));
    
    printf("%s success\n", __FUNCTION__);
    return;
}

int 
main(int argc, char const *argv[]) {
    printf("%s\n", __FILE__);
    test1();    
    test2();    
    test3();    
    printf("\n");
    return 0;
}