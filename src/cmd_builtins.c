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
    char cmdline[maxline]; /* command line */
}jobs[maxjobs];

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
job_clear(struct job_t* job) {
    job->jid = 0;
    job->pid = 0;
    job->cmdline[0] = '\0';
    job->state = job_undef;
}

static int
max_jid() {
    int max = 0;
    for (int i = 0; i < maxjobs; i++) {
        if (jobs[i].jid > max) {
            max = jobs[i].jid;
        }
    }
    return max;
}

static void
job_delete_by_pid(pid_t pid) {
    if (pid < 1) {
        return;
    }
 
    for (int i = 0; i < maxjobs; i++) {
        if (jobs[i].pid == pid) {
            job_clear(&(jobs[i]));
            next_jid = max_jid() + 1;
            return;
        }
    }
}

static struct job_t*
get_job_by_pid(pid_t pid){
    for (int i = 0; i < maxjobs; i++) {
        if (jobs[i].pid == pid) {
            return &(jobs[i]);
        }
    }
    return NULL;
}

static pid_t
fgpid() {
    for (int i = 0; i < maxjobs; i++) {
        if (jobs[i].state == job_fg) {
            return jobs[i].pid;
        }
    }
    return 0;
}

void
job_add(pid_t pid, enum jobs_state  state, char* cmdline) {
    if (pid < 1) {
    return;
    }

    for (int i = 0; i < maxjobs; i++) {
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
waitfg(int pid){
    struct job_t* p = get_job_by_pid(pid);
    if (p == NULL) {
        return;
    }

    pid_t fg;
    fg = fgpid();
    while (fg == pid) {
        fg = fgpid();
    }
    // for (pid_t fg = fgpid();fg != pid; fg = fgpid()) {
    
    // }
    return;
}

void
init_jobs() {
    for (int i = 0; i < maxjobs; i++) {
        job_clear(&(jobs[i]));
    }
    next_jid = max_jid() + 1;
}

// builtin commands
builtins_handler get_builtin(char* args[]) {
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
    
    struct job_t* p = get_job_by_pid(pid);
    if (p == NULL || p->state == job_undef) {
        log_info("(%d): No such process\n", pid);
        return 0;
    }
    
    log_info("[%d] (%d) %s\n", p->jid, p->pid, p->cmdline);
    p->state = job_bg;
    Kill(-p->pid, SIGCONT);
    return 0;
}

static int 
do_jobs(char* args[]) {
    for (int i = 0; i < maxjobs; i++) {
        struct job_t* p = &(jobs[i]);
        if (p->state != job_undef) {
            log_info("[%d] (%d) ", p->jid, p->pid);
            switch (p->state) {
            case job_bg: log_info("Running"); break;
            case job_fg: log_info("Foreground "); break;
            case job_st: log_info("Stopped "); break;
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
    
    struct job_t* p = get_job_by_pid(pid);
    if (p == NULL || p->state == job_undef) {
        log_info("(%d): No such process\n", pid);
        return 0;
    }
    
    log_info("[%d] (%d) %s\n", p->jid, p->pid, p->cmdline);
    p->state = job_fg;
    Kill(-p->pid, SIGCONT);
    waitfg(pid);    
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
sigchld_handler(int sig) { 
    int old_error = errno;
    int wstatus;
    pid_t child_pid; 
    while((child_pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED)) > 0){
        if (WIFEXITED(wstatus)) {
            // normally exit
            // log_info("normally exit: delete job\n");
            job_delete_by_pid(child_pid);
        } else if (WIFSTOPPED(wstatus)) {
            // stop
            struct job_t * job_p = get_job_by_pid(child_pid);
            log_info("Job [%d] (%d) stopped by signal %d\n", job_p->jid, job_p->pid, SIGTSTP);
            job_p->state = job_st;
        } else if (WIFSIGNALED(wstatus)) {
            // killed by signal
            struct job_t* job_p = get_job_by_pid(child_pid);
            log_info("Job [%d] (%d) terminated by signal %d\n", job_p->jid, job_p->pid, SIGINT);

            job_delete_by_pid(child_pid);
        }
    }

    errno = old_error;
    return; 
}

void
sigint_handler(int sig) {
    pid_t fore_pid = fgpid();
    if (fore_pid == 0) {
        log_info("\n$ ");
        fflush(stdout);
        return;
    }

    Kill(-fore_pid, sig);
    return;
}

void
sigtstp_handler(int sig) {
    pid_t fore_pid = fgpid();
    if (fore_pid == 0) {
        return;
    }

    Kill(-fore_pid, sig);
    return;
}

void
init_signal() {
    Signal(SIGINT, sigint_handler);   
    Signal(SIGTSTP, sigtstp_handler); 
    Signal(SIGCHLD, sigchld_handler);

    // Signal(SIGQUIT, sigquit_handler);
}
