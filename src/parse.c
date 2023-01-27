#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
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
    struct cmd* objects;
}parser;

/*
****************************
* function signature
****************************
*/
static struct cmd* parse();

/*
****************************
* function declaration
****************************
*/
#define ALLOCATE_OBJ(type, cmdtype) \
    (type*)allocateObject(sizeof(type), cmdtype)

static struct cmd*
allocateObject(int size, cmdtype type) {
    struct cmd* cmd = (struct cmd*)malloc(size);
    memset(cmd, 0, size);

    cmd->type = type;
    cmd->next = parser.objects;
    parser.objects = cmd;

    return cmd;
}

static struct cmd*
allocateMemCmdExec(void) {
    struct cmd_exec* cmd;
    cmd = ALLOCATE_OBJ(struct cmd_exec, cmdtype_exec);

    return (struct cmd*)cmd; 
}

static struct cmd*
allocateMemCmdRedir(struct cmd* subcmd, char* file, char* efile, int mode, int fd) {
    struct cmd_redir* cmd;
    cmd = ALLOCATE_OBJ(struct cmd_redir, cmdtype_redir);

    cmd->cmd = subcmd;
    cmd->file = file;
    cmd->efile = efile;
    cmd->mode = mode;
    cmd->fd = fd;
    return (struct cmd*)cmd; 
}

static struct cmd*
allocateMemCmdPipe(struct cmd* left, struct cmd* right) {
    struct cmd_pipe* cmd;
    cmd = ALLOCATE_OBJ(struct cmd_pipe, cmdtype_pipe);
    
    cmd->left = left;
    cmd->right = right;

    return (struct cmd*)cmd;
}

static struct cmd*
allocateMemCmdBack(struct cmd* subcmd) {
    struct cmd_back* cmd;
    cmd = ALLOCATE_OBJ(struct cmd_back, cmdtype_back);
    
    cmd->cmd = subcmd;
    
    return (struct cmd*)cmd;
}

static struct cmd*
allocateMemCmdList(struct cmd* left, struct cmd* right) {
    struct cmd_list* cmd;
    
    cmd = ALLOCATE_OBJ(struct cmd_list, cmdtype_list);
    
    cmd->left = left;
    cmd->right = right;

    return (struct cmd*)cmd;
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

static struct cmd*
parseRedirs(struct cmd* cmd) {
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


struct cmd*
parseBlock() {
    struct cmd* cmd = parse();
    consume(TokenTypeRightParen, "Expect ')' after block");

    cmd = parseRedirs(cmd);
    return cmd;
}

struct cmd*
parseExec() {
    struct cmd_exec* ecmd;

    ecmd = (struct cmd_exec*)allocateMemCmdExec();
    
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

        ecmd = (struct cmd_exec*)(parseRedirs((struct cmd*)ecmd));
    }

    ecmd->argv[argc] = NULL;
    ecmd->argc = argc;

    return (struct cmd*)ecmd;
}

struct cmd*
parsePipe() {
    struct cmd *cmd;
    cmd = parseExec();
    if (match(TokenTypePipe)) {
        cmd = allocateMemCmdPipe(cmd, parsePipe());
    }

    return cmd;
}

static struct cmd*
parse() {
    struct cmd* cmd;
    cmd = parsePipe();
    if (match(TokenTypeBackTask)) {
        cmd = allocateMemCmdBack(cmd);
    }

    if (match(TokenTypeSemicolon)) {
        cmd = allocateMemCmdList(cmd, parse());
    }

    if (match(TokenTypeEOF)) {
        return cmd;
    }

    // doesn't arrive here
    return NULL;
}

// NUL-terminate all the counted strings.
static struct cmd* 
NullTerminate(struct cmd* cmd) {
  int i;
  struct cmd_back* bcmd;
  struct cmd_exec* ecmd;
  struct cmd_list* lcmd;
  struct cmd_pipe* pcmd;
  struct cmd_redir* rcmd;

  if (cmd == NULL) {
    return NULL;
  }

  switch (cmd->type) {
    case cmdtype_exec:
      ecmd = (struct cmd_exec*)cmd;
      for (i = 0; ecmd->argv[i]; i++){
       *ecmd->eargv[i] = '\0';
      }
      break;

    case cmdtype_redir:
      rcmd = (struct cmd_redir*)cmd;
      NullTerminate(rcmd->cmd);
      *rcmd->efile = '\0';
      break;

    case cmdtype_pipe:
      pcmd = (struct cmd_pipe*)cmd;
      NullTerminate(pcmd->left);
      NullTerminate(pcmd->right);
      break;

    case cmdtype_list:
      lcmd = (struct cmd_list*)cmd;
      NullTerminate(lcmd->left);
      NullTerminate(lcmd->right);
      break;

    case cmdtype_back:
      bcmd = (struct cmd_back*)cmd;
      NullTerminate(bcmd->cmd);
      break;
  }
  return cmd;
}

static void
freeObjects() {
    struct cmd* p = parser.objects;
    while (p != NULL) {
        parser.objects = p->next;
        free(p);
        p = parser.objects;
    }
}

struct cmd*
cmdParse(char* buf) {
    // init
    scannerInit(buf);
    freeObjects();
    parser.error = false;

    struct cmd *cmd = NULL;
    
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
