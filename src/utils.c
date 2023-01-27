#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "parse.h"
#include "wrapper.h"

void
panic(char* msg) {
    log_info("panic: %s\n", msg);
    exit(-1);
}

void 
print_arr(char* prefix, char* array[], int argc) {
    log_info("%-8s", prefix);
    for (int i = 0; i < argc; i++) {
        log_info("[%s] ", array[i]);
    }
    log_info("\n");
}

void
print_cmd(struct Cmd* cmd) {
    struct CmdBack *bcmd;
    struct CmdExec *ecmd;
    struct CmdList *lcmd;
    struct CmdPipe *pcmd;
    struct CmdRedir *rcmd;
    struct CmdAnd *acmd;

    switch (cmd->type) {
        case CmdTypeExec:{
            ecmd = (struct CmdExec*)cmd;
            print_arr("exec:", ecmd->argv, ecmd->argc);
            break;
        } case CmdTypePipe:{
            pcmd = (struct CmdPipe*)cmd;
            log_info("===== pipe left =====\n");
            print_cmd(pcmd->left);
            log_info("===== pipe right =====\n");
            print_cmd(pcmd->right);
            break;
        } case CmdTypeBack:{
            log_info("===== back cmd =====\n");
            bcmd = (struct CmdBack*)cmd;
            print_cmd(bcmd->cmd);
            break;
        } case CmdTypeList: {
            lcmd = (struct CmdList*)cmd;
            log_info("===== list left =====\n");
            print_cmd(lcmd->left);
            log_info("===== list right =====\n");
            print_cmd(lcmd->right);
            break;
        } case CmdTypeRedir: {
            rcmd = (struct CmdRedir*)cmd;
            log_info("===== redir cmd =====\n");
            print_cmd(rcmd->cmd);
            log_info("mode: [%d] fd: [%d] file: [%s]\n", rcmd->mode, rcmd->fd, rcmd->file);
            break;
        } case CmdTypeAnd: {
            acmd = (struct CmdAnd*)cmd;
            log_info("===== and left =====\n");
            print_cmd(acmd->left);
            log_info("===== and right =====\n");
            print_cmd(acmd->right);
            break;
        }
    }

}

void
print_diff_exit(struct Cmd* cmd, char** expected, int argc) {
    log_info("\n=== parse cmd result ===\n");
    print_cmd(cmd);
    log_info("\n=== expect result ===\n");
    print_arr("expect:", expected, argc);
    exit(-1);
}

bool
equal_string(char* s, char* d) {
    if (strcmp(s, d) != 0) {
        log_info("test equal string\n");
        log_info("source: [%s]\nexpect: [%s]\n", s, d);
        return false;
    }
    return true;
}