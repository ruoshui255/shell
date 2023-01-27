#ifndef BUILTINS
#define BUILTINS
#include <sys/types.h>

typedef int (*builtins_handler)(char * args[]);

enum {MaxLine=128, MaxJobs=20};
typedef enum {JobStateUndefine, JobStateBG, JobStateFG, JobStateStop}JobState;

builtins_handler builtinGet(char* args[]);

void signalInit();
void jobInit();
void waitFG(pid_t pid);
void jobAdd(pid_t pid, JobState state, char* cmdline);

#endif