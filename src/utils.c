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
print_cmd(struct cmd* cmd) {
    struct cmd_back *bcmd;
    struct cmd_exec *ecmd;
    struct cmd_list *lcmd;
    struct cmd_pipe *pcmd;
    struct cmd_redir *rcmd;

    switch (cmd->type) {
        case cmdtype_exec:{
            ecmd = (struct cmd_exec*)cmd;
            print_arr("exec:", ecmd->argv, ecmd->argc);
            break;
        } case cmdtype_pipe:{
            pcmd = (struct cmd_pipe*)cmd;
            log_info("===== pipe left =====\n");
            print_cmd(pcmd->left);
            log_info("===== pipe right =====\n");
            print_cmd(pcmd->right);
            break;
        } case cmdtype_back:{
            log_info("===== back cmd =====\n");
            bcmd = (struct cmd_back*)cmd;
            print_cmd(bcmd->cmd);
            break;
        } case cmdtype_list: {
            lcmd = (struct cmd_list*)cmd;
            log_info("===== list left =====\n");
            print_cmd(lcmd->left);
            log_info("===== list right =====\n");
            print_cmd(lcmd->right);
            break;
        } case cmdtype_redir: {
            rcmd = (struct cmd_redir*)cmd;
            log_info("===== redir cmd =====\n");
            print_cmd(rcmd->cmd);
            log_info("mode: [%d] fd: [%d] file: [%s]\n", rcmd->mode, rcmd->fd, rcmd->file);
            break;
        }
    }

}

void
print_diff_exit(struct cmd* cmd, char** expected, int argc) {
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