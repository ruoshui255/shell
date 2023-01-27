#ifndef _WRAPPER
#define _WRAPPER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

typedef void handler_t(int);
#define log_info(...) (fprintf(stderr, __VA_ARGS__));

pid_t wrapperFork();
int wrapperDup(int fd);
void wrapperClose(int fd);
int wrapperOpen(const char* pathname, int flags);
pid_t wrapperWait(int *wstatus);
void wrapperExecvp(const char *file, char *const argv[]);
int wrapperKill(pid_t pid, int sig);
void *wrapperMalloc(size_t size);
handler_t *wrapperSignal(int signum, handler_t *handler);
#endif