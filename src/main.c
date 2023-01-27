#include <stdbool.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/types.h>

#include "cmd_builtins.h"
#include "wrapper.h"
#include "parse.h"

// gloabl variable
extern struct cmd* cmd_parse(char* buf);
extern struct cmd* OJBECTS;

int
getcmd(char *buf, int nbuf) {
    log_info("$ ");
    fflush(stderr);
    memset(buf, 0, nbuf);
    
    fgets(buf, nbuf, stdin);
    // when EOF, it doesn't change the buf(initialization)
    // so buf[0] is zero
    if(buf[0] == 0){ // EOF
        return -1;
    }
    
    return 0;
}

// Execute cmd.  Never returns.
void
run_cmd(struct cmd *cmd) {
    int p[2];
    struct cmd_back *bcmd;
    struct cmd_exec *ecmd;
    struct cmd_list *lcmd;
    struct cmd_pipe *pcmd;
    struct cmd_redir *rcmd;

    if (cmd == 0)
        exit(1);

    // print_cmd(cmd);
    switch (cmd->type) {
    default:
      panic("runcmd");

    case cmdtype_exec:
        ecmd = (struct cmd_exec *)cmd;
      if (ecmd->argv[0] == 0)
        exit(1);
      Execvp(ecmd->argv[0], ecmd->argv);
      break;

    case cmdtype_redir:
      rcmd = (struct cmd_redir *)cmd;
      Close(rcmd->fd);
      Open(rcmd->file, rcmd->mode);
      run_cmd(rcmd->cmd);
      break;

    case cmdtype_list:
        lcmd = (struct cmd_list *)cmd;
        if (Fork() == 0)
            run_cmd(lcmd->left);
        Wait(0);
        run_cmd(lcmd->right);
      break;

    case cmdtype_pipe:
      pcmd = (struct cmd_pipe *)cmd;
      if (pipe(p) < 0)
        panic("pipe");
      if (Fork() == 0) {
        Close(1);
        Dup(p[1]);
        Close(p[0]);
        Close(p[1]);
        run_cmd(pcmd->left);
      }
      if (Fork() == 0) {
        Close(0);
        Dup(p[0]);
        Close(p[0]);
        Close(p[1]);
        run_cmd(pcmd->right);
      }
      Close(p[0]);
      Close(p[1]);
      Wait(0);
      Wait(0);
      break;

    case cmdtype_back:
      bcmd = (struct cmd_back *)cmd;
      // if(Fork() == 0)
      run_cmd(bcmd->cmd);
      break;
    }
    exit(0);
}


void
init() {
    init_signal();
    init_jobs();

    OJBECTS->next = NULL;
}

int 
main(int argc, char const *argv[]) {
    init();

    static char buf[100];
    int fd;

  // Ensure that three file descriptors are Open.

  // Read and run input commands.
    while((getcmd(buf, sizeof(buf))) >= 0){
        
        struct cmd* cmd = cmd_parse(buf);
        if (parse_error) {
            parse_error = false;
            continue;
        }

        // builtin method
        if (cmd->type == cmdtype_exec) {
            struct cmd_exec* ecmd = (struct cmd_exec*) cmd;

            // skip whitespace
            if (ecmd->argc == 0) {
                continue;
            }            

            builtins_handler fun = get_builtin(ecmd->argv);
            if (fun) {
                fun(ecmd->argv);
                continue;
            }
        }

        // fork and execute
        sigset_t mask, prev;
        sigaddset(&mask, SIGCHLD);
        sigprocmask(SIG_BLOCK, &mask, &prev);  

        pid_t pid = Fork();
        if(pid == 0) {
            sigprocmask(SIG_SETMASK, &prev , NULL);
            setpgid(0, 0);        

            run_cmd(cmd);
            exit(-1);
        }

        enum jobs_state state = cmd->type == cmdtype_back? job_bg : job_fg;
        job_add(pid, state, buf);
        sigprocmask(SIG_SETMASK, &prev , NULL);
        
        if (state == job_bg) {
            log_info("bg (%d) %s\n", pid, buf);
        } else {
            waitfg(pid);
        }
    }
    return 0;
}