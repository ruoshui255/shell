#ifndef PARSE
#define PARSE

#include <stdbool.h>

#define MAXARGS 10

typedef enum{
    cmdtype_exec,
    cmdtype_redir,
    cmdtype_pipe,
    cmdtype_list,
    cmdtype_back,
}cmdtype;

struct cmd {
    struct cmd* next;
    cmdtype type;
};

struct cmd_exec {
    struct cmd* next;
    cmdtype type;
    char* argv[MAXARGS];
    char* eargv[MAXARGS];
    int argc;
};

struct cmd_redir {
    struct cmd* next;
    cmdtype type;
    struct cmd* cmd;
    char* file;
    int mode;
    int fd;
};

struct cmd_pipe {
    struct cmd* next;
    cmdtype type;
    struct cmd* left;
    struct cmd* right;
};

struct cmd_list {
    struct cmd* next;
    cmdtype type;
    struct cmd* left;
    struct cmd* right;
};

struct cmd_back {
    struct cmd* next;
    cmdtype type;
    struct cmd* cmd;
};

/* ======== utils ===== */
void panic(char* msg);
void print_arr(char* prefix, char* array[], int argc);
void print_cmd(struct cmd* cmd);
void print_diff_exit(struct cmd* cmd, char** expected, int argc);
bool equal_string(char* s, char* d);

/* ======== parse ===== */
struct cmd* parse_line();
struct cmd* parse_pipe();
struct cmd* parse_exec();

#endif