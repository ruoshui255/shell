#ifndef BUILTINS
#define BUILTINS
#include <sys/types.h>

typedef int (*builtins_handler)(char * args[]);

enum foo {maxline=128, maxjobs=20};
enum jobs_state {job_undef, job_bg, job_fg, job_st};

builtins_handler get_builtin(char* args[]);

void init_signal();
void init_jobs();
void waitfg(pid_t pid);
void job_add(pid_t pid, enum jobs_state state, char* cmdline);

#endif