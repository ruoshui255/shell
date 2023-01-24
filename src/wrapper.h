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

pid_t Fork();
int Dup(int fd);
void Close(int fd);
int Open(const char* pathname, int flags);
pid_t Wait(int *wstatus);
void Execvp(const char *file, char *const argv[]);
int Kill(pid_t pid, int sig);
handler_t *Signal(int signum, handler_t *handler);
#endif