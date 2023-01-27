#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

#include "wrapper.h"
#include "parse.h"

static void
unixError(char* msg) {
    log_info("%s : %s\n", msg, strerror(errno));
    exit(-1);
}

pid_t
wrapperFork() {
    pid_t pid = fork();

    if (pid < 0) {
        unixError("fork error");
    }

    return pid;
}


int 
wrapperDup(int fd){
    int state = dup(fd);
    if (state < 0) {
        unixError("dup error");
    }

    return 0;
}


void
wrapperClose(int fd) {
    int ret = close(fd);
    if (ret < 0) {
        unixError("close error");
    }

    return;
}

int 
wrapperOpen(const char* pathname, int flags) {
    int fd = open(pathname, flags, S_IRWXU);
    if (fd < 0) {
        fprintf(stderr, "open (%s) error : %s\n", pathname, strerror(errno));
        exit(-1);
    }

    return fd;
}

pid_t
wrapperWait(int *wstatus) {
    pid_t child_term = wait(wstatus);
    if (child_term < 0) {
        unixError("wait error");
    }
    
    return child_term;
}

void
wrapperExecvp(const char *file, char *const argv[]) {
    int ret = execvp(file, argv);
    if (ret < 0) {
        log_info("execvp %s error : %s\n", file, strerror(errno));

        exit(45);
    }
}

int
wrapperKill(pid_t pid, int sig) {
    int res = kill(pid, sig);
    if (res < 0) {
        unixError("kill error");
    }
    
    return res;
}

void*
wrapperMalloc(size_t size) {
    void* res = malloc(size);
    if (res == NULL) {
        unixError("kill error");
    }
    
    return res;
}


handler_t *
wrapperSignal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0) unixError("Signal error");
    return (old_action.sa_handler);
}