#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "parse.h"

extern struct cmd* objects;

#define ALLOCATE_OBJ(type, cmdtype) \
    (type*)allocateObject(sizeof(type), cmdtype)


struct cmd*
allocateObject(size_t size, cmdtype type) {
    struct cmd* cmd = (struct cmd*)malloc(size);
    memset(cmd, 0, size);

    cmd->type = type;
    cmd->next = objects->next;
    objects->next = cmd;

    return cmd;
}

struct cmd*
allocate_mem_cmd_exec(void) {
    struct cmd_exec* cmd;
    cmd = ALLOCATE_OBJ(struct cmd_exec, cmdtype_exec);

    return (struct cmd*)cmd; 
}

struct cmd*
allocate_mem_cmd_redir(struct cmd* subcmd, char* file, int mode, int fd) {
    struct cmd_redir* cmd;
    cmd = ALLOCATE_OBJ(struct cmd_redir, cmdtype_redir);

    cmd->cmd = subcmd;
    cmd->file = file;
    cmd->mode = mode;
    cmd->fd = fd;
    return (struct cmd*)cmd; 
}

struct cmd*
allocate_mem_cmd_pipe(struct cmd* left, struct cmd* right) {
    struct cmd_pipe* cmd;
    cmd = ALLOCATE_OBJ(struct cmd_pipe, cmdtype_pipe);
    
    cmd->left = left;
    cmd->right = right;

    return (struct cmd*)cmd;
}

struct cmd*
allocate_mem_cmd_back(struct cmd* subcmd) {
    struct cmd_back* cmd;
    cmd = ALLOCATE_OBJ(struct cmd_back, cmdtype_back);
    
    cmd->cmd = subcmd;
    
    return (struct cmd*)cmd;
}

struct cmd*
allocate_mem_cmd_list(struct cmd* left, struct cmd* right) {
    struct cmd_list* cmd;
    
    cmd = ALLOCATE_OBJ(struct cmd_list, cmdtype_list);
    
    cmd->left = left;
    cmd->right = right;

    return (struct cmd*)cmd;
}

struct cmd*
parse_redirs(struct cmd* cmd) {
    while (scanner_peek_equal("<+>")) {
        char c = scanner_next();
        char* file = scanner_get_token();
        // todo check file exist

        switch (c) {
        case '<': cmd = allocate_mem_cmd_redir(cmd, file, O_RDONLY, 0);break;
        case '>': 
            cmd = allocate_mem_cmd_redir(cmd, file, O_WRONLY|O_CREAT|O_TRUNC, 1);
            break;
        case '+': cmd = allocate_mem_cmd_redir(cmd, file, O_WRONLY|O_CREAT| O_APPEND, 1);break;
        }
    }
    return cmd;
}


struct cmd*
parse_block() {
    scanner_consume('(', "parse block");
    struct cmd* cmd = parse_line();
    scanner_consume(')', "syntax-missing");

    cmd = parse_redirs(cmd);
    return cmd;
}

struct cmd*
parse_exec() {
    struct cmd_exec* cmd;
    struct cmd* ret;

    ret = allocate_mem_cmd_exec();
    cmd = (struct cmd_exec*)ret;

    // parse block todo
    if (scanner_peek_equal("(")) {
        return parse_block();
    }

    int argc = 0;
    while (scanner_has_next() && (!scanner_peek_equal("|)&;"))) {
        char* token = scanner_get_token();
        cmd->argv[argc] = token;
        argc++;
        // parse redir todo
        ret = parse_redirs(ret);
    }

    cmd->argv[argc] = NULL;
    cmd->argc = argc;

    return ret;
}

struct cmd*
parse_pipe() {
    struct cmd *cmd;
    cmd = parse_exec();
    if (scanner_peek_equal("|")) {
        scanner_consume('|', "parse exe | error");
        cmd = allocate_mem_cmd_pipe(cmd, parse_pipe());
    }

    return cmd;
}

struct cmd*
parse_line() {
    struct cmd* cmd;

    cmd = parse_pipe();
    if (scanner_peek_equal("&")) {
        scanner_get_token();
        cmd = allocate_mem_cmd_back(cmd);
    }

    if (scanner_peek_equal(";")) {
        scanner_get_token();
        cmd = allocate_mem_cmd_list(cmd, parse_line());
    }
    return cmd;
}

struct cmd*
cmd_parse(char* buf) {
    scanner_init(buf);
    struct cmd* cmd = parse_line();
    return cmd;
}