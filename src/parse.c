#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "parse.h"
#include "scanner.h"
#include "wrapper.h"

/*
****************************
* global variable
****************************
*/
static struct {
    Token previous;
    Token current;
    bool error;
    struct Cmd* objects;
}parser;

/*
****************************
* function signature
****************************
*/
static struct Cmd* parse();

/*
****************************
* function declaration
****************************
*/
#define ALLOCATE_OBJ(type, cmdtype) \
    (type*)allocateObject(sizeof(type), cmdtype)

static struct Cmd*
allocateObject(int size, CmdType type) {
    struct Cmd* cmd = (struct Cmd*)wrapperMalloc(size);
    memset(cmd, 0, size);

    cmd->type = type;
    cmd->next = parser.objects;
    parser.objects = cmd;

    return cmd;
}

static struct Cmd*
allocateMemCmdExec(void) {
    struct CmdExec* cmd;
    cmd = ALLOCATE_OBJ(struct CmdExec, CmdTypeExec);

    return (struct Cmd*)cmd; 
}

static struct Cmd*
allocateMemCmdRedir(struct Cmd* subcmd, char* file, char* efile, int mode, int fd) {
    struct CmdRedir* cmd;
    cmd = ALLOCATE_OBJ(struct CmdRedir, CmdTypeRedir);

    cmd->cmd = subcmd;
    cmd->file = file;
    cmd->efile = efile;
    cmd->mode = mode;
    cmd->fd = fd;
    return (struct Cmd*)cmd; 
}

static struct Cmd*
allocateMemCmdPipe(struct Cmd* left, struct Cmd* right) {
    struct CmdPipe* cmd;
    cmd = ALLOCATE_OBJ(struct CmdPipe, CmdTypePipe);
    
    cmd->left = left;
    cmd->right = right;

    return (struct Cmd*)cmd;
}

static struct Cmd*
allocateMemCmdBack(struct Cmd* subcmd) {
    struct CmdBack* cmd;
    cmd = ALLOCATE_OBJ(struct CmdBack, CmdTypeBack);
    
    cmd->cmd = subcmd;
    
    return (struct Cmd*)cmd;
}

static struct Cmd*
allocateMemCmdList(struct Cmd* left, struct Cmd* right) {
    struct CmdList* cmd;
    
    cmd = ALLOCATE_OBJ(struct CmdList, CmdTypeList);
    
    cmd->left = left;
    cmd->right = right;

    return (struct Cmd*)cmd;
}

static struct Cmd*
allocateMemCmdAnd(struct Cmd* left, struct Cmd* right) {
    struct CmdAnd* cmd;
    
    cmd = ALLOCATE_OBJ(struct CmdAnd, CmdTypeAnd);
    
    cmd->left = left;
    cmd->right = right;

    return (struct Cmd*)cmd;
}

static void
advance() {
    parser.previous = parser.current;
    for (;;) {
        parser.current =  scannerGetToken();
        if (parser.current.type != TokenTypeError) {
            break;
        }
    }
}

static bool
check(TokenType type) {
    return parser.current.type == type;
}

static bool
match(TokenType type) {
    if (!check(type)) {
        return false;
    } else {
        advance();
        return true;
    }
}

static void 
consume(TokenType type, char* msg_err) {
    if (parser.error) {
        return;
    }

    Token t = scannerGetToken();
    if (t.type != type) {
        log_info("%s\n", msg_err);
        parser.error = true;
    }
    return;
}
static bool
matchRedir() {
    bool redir = match(TokenTypeRedirectionRead) || match(TokenTypeRedirectionWrite) || match(TokenTypeRedirectionWriteAppend);
    return redir;
}

static struct Cmd*
parseRedirs(struct Cmd* cmd) {
    while (matchRedir()) {
        Token t = parser.previous;
        Token file = parser.current;
        char* efile = file.start + file.length;
        
        advance();
        switch (t.type) {
            case TokenTypeRedirectionRead: 
                cmd = allocateMemCmdRedir(cmd, file.start, efile, O_RDONLY, 0);break;
            case TokenTypeRedirectionWrite: 
                cmd = allocateMemCmdRedir(cmd, file.start, efile, O_WRONLY|O_CREAT|O_TRUNC, 1);
                break;
            case TokenTypeRedirectionWriteAppend:
                cmd = allocateMemCmdRedir(cmd, file.start, efile, O_WRONLY|O_CREAT| O_APPEND, 1);break;
            default:
                log_info("error parse redir: %d\n", t.type);
            }
        }
    return cmd;
}


struct Cmd*
parseBlock() {
    struct Cmd* cmd = parse();
    consume(TokenTypeRightParen, "Expect ')' after block");

    cmd = parseRedirs(cmd);
    return cmd;
}

struct Cmd*
parseExec() {
    struct CmdExec* ecmd;

    ecmd = (struct CmdExec*)allocateMemCmdExec();
    
    if (match(TokenTypeLeftParen)) {
        return parseBlock();
    }

    int argc = 0;
    while (check(TokenTypeArg)) {
        advance();
        Token token = parser.previous;
        ecmd->argv[argc] = token.start;
        ecmd->eargv[argc] = token.start + token.length;
        argc++;

        ecmd = (struct CmdExec*)(parseRedirs((struct Cmd*)ecmd));
    }

    ecmd->argv[argc] = NULL;
    ecmd->argc = argc;

    return (struct Cmd*)ecmd;
}

struct Cmd*
parsePipe() {
    struct Cmd *cmd;
    cmd = parseExec();
    if (match(TokenTypePipe)) {
        cmd = allocateMemCmdPipe(cmd, parsePipe());
    }

    return cmd;
}

static struct Cmd*
parse() {
    struct Cmd* cmd;
    cmd = parsePipe();
    if (match(TokenTypeBackTask)) {
        cmd = allocateMemCmdBack(cmd);
    }

    if (match(TokenTypeAnd)) {
        cmd = allocateMemCmdAnd(cmd, parse());
    }

    if (match(TokenTypeSemicolon)) {
        cmd = allocateMemCmdList(cmd, parse());
    }

    if (match(TokenTypeEOF)) {
        return cmd;
    }

    // doesn't arrive here
    log_info("parse error: type %d\n", parser.current.type);
    return NULL;
}

// NUL-terminate all the counted strings.
static struct Cmd* 
NullTerminate(struct Cmd* cmd) {
    int i;
    struct CmdBack* bcmd;
    struct CmdExec* ecmd;
    struct CmdList* lcmd;
    struct CmdPipe* pcmd;
    struct CmdRedir* rcmd;
    struct CmdAnd* acmd;

    if (cmd == NULL) {
        return NULL;
    }

    switch (cmd->type) {
        case CmdTypeExec:
            ecmd = (struct CmdExec*)cmd;
            for (i = 0; ecmd->argv[i]; i++){
                *ecmd->eargv[i] = '\0';
            }
            break;
        case CmdTypeRedir:
            rcmd = (struct CmdRedir*)cmd;
            NullTerminate(rcmd->cmd);
            *rcmd->efile = '\0';
            break;
        case CmdTypePipe:
            pcmd = (struct CmdPipe*)cmd;
            NullTerminate(pcmd->left);
            NullTerminate(pcmd->right);
            break;
        case CmdTypeList:
            lcmd = (struct CmdList*)cmd;
            NullTerminate(lcmd->left);
            NullTerminate(lcmd->right);
            break;
        case CmdTypeBack:
            bcmd = (struct CmdBack *)cmd;
            NullTerminate(bcmd->cmd);
            break;
        case CmdTypeAnd:
            acmd = (struct CmdAnd*)cmd;
            NullTerminate(acmd->left);
            NullTerminate(acmd->right);
            break;
    }
    return cmd;
}

static void
freeObjects() {
    struct Cmd* p = parser.objects;
    while (p != NULL) {
        parser.objects = p->next;
        free(p);
        p = parser.objects;
    }
}

struct Cmd*
cmdParse(char* buf) {
    // init
    scannerInit(buf);
    freeObjects();
    parser.error = false;

    struct Cmd *cmd = NULL;
    
    // parse
    advance();
    if (!match(TokenTypeEOF)) {
        cmd = parse();
        if (parser.error) {
            cmd = NULL;
        } else {
            NullTerminate(cmd);
        }
    }

    return cmd;
}
