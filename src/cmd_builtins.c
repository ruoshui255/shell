#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>

#include "wrapper.h"
#include "cmd_builtins.h"

static int do_exit(char* args[]);
static int do_cd(char* args[]);
static int do_fg(char* args[]);
static int do_bg(char* args[]);
static int do_jobs(char* args[]);


static int next_jid = 1;

static struct job_t {      /* The job struct */
    pid_t pid;             /* job PID */
    int jid;               /* job ID [1, 2, ...] */
    int state;             /* UNDEF, BG, FG, or ST */
    char cmdline[MaxLine]; /* command line */
}jobs[MaxJobs];

static struct builtin_cmd {
    char *name;
    builtins_handler handler;
} builtins[] = {
    {"exit", do_exit}, 
    {"cd", do_cd}, 
    {"fg", do_fg},
    {"bg", do_bg},
    {"jobs", do_jobs},
    {NULL, NULL}
};

static void
jobClear(struct job_t* job) {
    job->jid = 0;
    job->pid = 0;
    job->cmdline[0] = '\0';
    job->state = JobStateUndefine;
}

static int
jobMaxId() {
    int max = 0;
    for (int i = 0; i < MaxJobs; i++) {
        if (jobs[i].jid > max) {
            max = jobs[i].jid;
        }
    }
    return max;
}

static void
jobDeleteByPid(pid_t pid) {
    if (pid < 1) {
        return;
    }
 
    for (int i = 0; i < MaxJobs; i++) {
        if (jobs[i].pid == pid) {
            jobClear(&(jobs[i]));
            next_jid = jobMaxId() + 1;
            return;
        }
    }
}

static struct job_t*
jobGetByPid(pid_t pid){
    for (int i = 0; i < MaxJobs; i++) {
        if (jobs[i].pid == pid) {
            return &(jobs[i]);
        }
    }
    return NULL;
}

static pid_t
pidGetFG() {
    for (int i = 0; i < MaxJobs; i++) {
        if (jobs[i].state == JobStateFG) {
            return jobs[i].pid;
        }
    }
    return 0;
}

void
jobAdd(pid_t pid, JobState  state, char* cmdline) {
    if (pid < 1) {
    return;
    }

    for (int i = 0; i < MaxJobs; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = next_jid++;
            strcpy(jobs[i].cmdline, cmdline);
            // log_info("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            return;
        }
    }
}

void
waitFG(int pid){
    struct job_t* p = jobGetByPid(pid);
    if (p == NULL) {
        return;
    }

    pid_t fg;
    fg = pidGetFG();
    while (fg == pid) {
        fg = pidGetFG();
    }
    // for (pid_t fg = fgpid();fg != pid; fg = fgpid()) {
    
    // }
    return;
}

void
jobInit() {
    for (int i = 0; i < MaxJobs; i++) {
        jobClear(&(jobs[i]));
    }
    next_jid = jobMaxId() + 1;
}

// builtin commands
builtins_handler builtinGet(char* args[]) {
    int i = 0;
    while (builtins[i].name != NULL) {
        int res = strcmp(builtins[i].name, args[0]);
        if (res == 0) {
            return builtins[i].handler;
        }
        i++;
    }

    return NULL;
}

static int 
do_bg(char* args[]) {
    if (args[1] == NULL) {
        log_info("Usage:bg %%job_id\n");
        return 0;
    }
    
    if (args[1][0] != '%') {
        log_info("<%s> format error\n", args[1]);
        log_info("Usage:bg %%job_id\n");
        return 0;
    }

    int pid = atoi(&(args[1][1]));
    if (pid == 0) {
        log_info("Usage:bg %%job_id\n");
    }
    
    struct job_t* p = jobGetByPid(pid);
    if (p == NULL || p->state == JobStateUndefine) {
        log_info("(%d): No such process\n", pid);
        return 0;
    }
    
    log_info("[%d] (%d) %s\n", p->jid, p->pid, p->cmdline);
    p->state = JobStateBG;
    wrapperKill(-p->pid, SIGCONT);
    return 0;
}

static int 
do_jobs(char* args[]) {
    for (int i = 0; i < MaxJobs; i++) {
        struct job_t* p = &(jobs[i]);
        if (p->state != JobStateUndefine) {
            log_info("[%d] (%d) ", p->jid, p->pid);
            switch (p->state) {
            case JobStateBG: log_info("Running"); break;
            case JobStateFG: log_info("Foreground "); break;
            case JobStateStop: log_info("Stopped "); break;
            default: log_info("error ");break;
            }
        
            log_info("%s \n", p->cmdline);
        }
    }
    return 0;
}

static int 
do_fg(char* args[]) {
    if (args[1] == NULL) {
        log_info("Usage:fg %%job_id\n");
        return 0;
    }

    if (args[1][0] != '%') {
        log_info("<%s> format error\n", args[1]);
        log_info("Usage:fg %%job_id\n");
        return 0;
    }

    int pid = atoi(&(args[1][1]));
    if (pid == 0) {
        log_info("Usage:fg %%job_id\n");
    }
    
    struct job_t* p = jobGetByPid(pid);
    if (p == NULL || p->state == JobStateUndefine) {
        log_info("(%d): No such process\n", pid);
        return 0;
    }
    
    log_info("[%d] (%d) %s\n", p->jid, p->pid, p->cmdline);
    p->state = JobStateFG;
    wrapperKill(-p->pid, SIGCONT);
    waitFG(pid);    
    return 0;
}

static int 
do_cd(char* args[]) {
    if (args[1] != NULL) {
        if (chdir(args[1]) != 0) {
            perror("cd error");
        }
    }
    return 0;
}

static int 
do_exit(char **args) {
    log_info("exit shell\n");
    exit(EXIT_SUCCESS);
    return 0;
}

// signal
void 
handlerSigChild(int sig) { 
    int old_error = errno;
    int wstatus;
    pid_t child_pid; 
    while((child_pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED)) > 0){
        if (WIFEXITED(wstatus)) {
            // normally exit
            log_info("normally exit: delete job\n");
            jobDeleteByPid(child_pid);
        } else if (WIFSTOPPED(wstatus)) {
            // stop
            struct job_t * job_p = jobGetByPid(child_pid);
            log_info("Job [%d] (%d) stopped by signal %d\n", job_p->jid, job_p->pid, SIGTSTP);
            job_p->state = JobStateStop;
        } else if (WIFSIGNALED(wstatus)) {
            // killed by signal
            struct job_t* job_p = jobGetByPid(child_pid);
            log_info("Job [%d] (%d) terminated by signal %d\n", job_p->jid, job_p->pid, SIGINT);

            jobDeleteByPid(child_pid);
        }
    }

    errno = old_error;
    return; 
}

void
handlerSigInt(int sig) {
    pid_t fore_pid = pidGetFG();
    if (fore_pid == 0) {
        log_info("\n$ ");
        fflush(stdout);
        return;
    }

    wrapperKill(-fore_pid, sig);
    return;
}

void
handlerSigStop(int sig) {
    pid_t fore_pid = pidGetFG();
    if (fore_pid == 0) {
        return;
    }

    wrapperKill(-fore_pid, sig);
    return;
}

void
signalInit() {
    wrapperSignal(SIGINT, handlerSigInt);   
    wrapperSignal(SIGTSTP, handlerSigStop); 
    wrapperSignal(SIGCHLD, handlerSigChild);

    // Signal(SIGQUIT, sigquit_handler);
}
