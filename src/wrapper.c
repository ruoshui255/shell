#include "wrapper.h"
#include "parse.h"
#include <stdio.h>

static void
unix_error(char* msg) {
    fprintf(stderr, "%s : %s\n", msg, strerror(errno));
    exit(-1);
}

pid_t
Fork() {
    pid_t pid = fork();

    if (pid < 0) {
        unix_error("fork error");
    }

    return pid;
}


int 
Dup(int fd){
    int state = dup(fd);
    if (state < 0) {
        unix_error("dup error");
    }

    return 0;
}


void
Close(int fd) {
    int ret = close(fd);
    if (ret < 0) {
        unix_error("close error");
    }

    return;
}

int 
Open(const char* pathname, int flags) {
    fprintf(stderr, "flags %d\n", flags);
    int fd = open(pathname, flags);
    if (fd < 0) {
        fprintf(stderr, "open (%s) error : %s\n", pathname, strerror(errno));
        exit(-1);
    }

    return fd;
}

pid_t
Wait(int *wstatus) {
    pid_t child_term = wait(wstatus);
    if (child_term < 0) {
        unix_error("wait error");
    }
    
    return child_term;
}

void
Execvp(const char *file, char *const argv[]) {
    int ret = execvp(file, argv);
    if (ret < 0) {
        fprintf(stderr, "execvp %s error : %s\n", file, strerror(errno));
    }
}


handler_t 
*Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0) unix_error("Signal error");
    return (old_action.sa_handler);
}