#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>

#include "cmd_builtins.h"
#include "wrapper.h"
#include "parse.h"


static int
getCmd(char *buf, int nbuf) {
    log_info("$ ");
    fflush(stderr);
    
    if(fgets(buf, nbuf, stdin) == NULL){
        return -1;
    }
    return 0;
}

// Execute cmd.  Never returns.
static void
runCmd(struct Cmd *cmd) {
    int p[2];
    struct CmdBack *bcmd;
    struct CmdExec *ecmd;
    struct CmdList *lcmd;
    struct CmdPipe *pcmd;
    struct CmdRedir *rcmd;
    struct CmdAnd *acmd;

    if (cmd == 0)
        exit(1);

    // print_cmd(cmd);
    switch (cmd->type) {
        default:
            panic("runcmd");
        case CmdTypeExec:
            ecmd = (struct CmdExec *)cmd;
            if (ecmd->argv[0] == 0)
                exit(0);
            wrapperExecvp(ecmd->argv[0], ecmd->argv);
            break;
        case CmdTypeRedir:
            rcmd = (struct CmdRedir *)cmd;
            wrapperClose(rcmd->fd);
            wrapperOpen(rcmd->file, rcmd->mode);
            runCmd(rcmd->cmd);
            break;
        case CmdTypeList:
            lcmd = (struct CmdList *)cmd;
            if (wrapperFork() == 0)
                runCmd(lcmd->left);
            wrapperWait(0);
            runCmd(lcmd->right);
            break;
        case CmdTypePipe:
            pcmd = (struct CmdPipe *)cmd;
            if (pipe(p) < 0)
                panic("pipe");

            if (wrapperFork() == 0) {
                wrapperClose(1);
                wrapperDup(p[1]);
                wrapperClose(p[0]);
                wrapperClose(p[1]);
                runCmd(pcmd->left);
            }

            if (wrapperFork() == 0) {
                wrapperClose(0);
                wrapperDup(p[0]);
                wrapperClose(p[0]);
                wrapperClose(p[1]);
                runCmd(pcmd->right);
            }
            wrapperClose(p[0]);
            wrapperClose(p[1]);
            wrapperWait(0);
            wrapperWait(0);
            break;
        case CmdTypeBack:
            bcmd = (struct CmdBack *)cmd;
            runCmd(bcmd->cmd);
            break;
        case CmdTypeAnd:
            acmd = (struct CmdAnd*)cmd;
            if (wrapperFork() == 0) {
                runCmd(acmd->left);
            }

            int wstatus;
            wrapperWait(&wstatus);
            if (WIFEXITED(wstatus) && (WEXITSTATUS(wstatus) == 0)) {
                runCmd(acmd->right);
            }
        }
    exit(0);
}


static void
init() {
    signalInit();
    jobInit();
}

int 
main(int argc, char const *argv[]) {
    init();
    static char buf[MaxLine];
    int fd;
  // Ensure that three file descriptors are Open.

  // Read and run input commands.
    while((getCmd(buf, sizeof(buf))) >= 0){
        struct Cmd* cmd = cmdParse(buf);
        if (cmd == NULL) {
            // printf("cmd null\n");
            continue;
        }

        // builtin method
        if (cmd->type == CmdTypeExec) {
            struct CmdExec* ecmd = (struct CmdExec*) cmd;

            builtins_handler fun = builtinGet(ecmd->argv);
            if (fun) {
                fun(ecmd->argv);
                continue;
            }
        }

        // fork and execute
        sigset_t mask, prev;
        Sigaddset(&mask, SIGCHLD);
        Sigprocmask(SIG_BLOCK, &mask, &prev);  

        pid_t pid = wrapperFork();
        if(pid == 0) {
            Sigprocmask(SIG_SETMASK, &prev , NULL);
            setpgid(0, 0);        

            runCmd(cmd);
            exit(-1);
        }

        JobState state = cmd->type == CmdTypeBack? JobStateBG : JobStateFG;
        jobAdd(pid, state, buf);
        Sigprocmask(SIG_SETMASK, &prev , NULL);
        
        if (state == JobStateBG) {
            log_info("bg (%d) %s\n", pid, buf);
        } else {
            waitFG(pid);
        }
    }
    return 0;
}