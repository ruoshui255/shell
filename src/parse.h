#ifndef PARSE
#define PARSE

#include <stdbool.h>

enum cmdtype {
    cmdtype_exec,
    cmdtype_redir,
    cmdtype_pipe,
    cmdtype_list,
    cmdtype_back,
};

#define MAXARGS 10


struct cmd {
    int type;
};

struct cmd_exec {
    int type;
    char* argv[MAXARGS];
    int argc;
};

struct cmd_redir {
    int type;
    struct cmd* cmd;
    char* file;
    int mode;
    int fd;
};

struct cmd_pipe {
    int type;
    struct cmd* left;
    struct cmd* right;
};

struct cmd_list {
    int type;
    struct cmd* left;
    struct cmd* right;
};

struct cmd_back {
    int type;
    struct cmd* cmd;
};

/* ======== utils ===== */
void panic(char* msg);
void print_arr(char* prefix, char* array[], int argc);
void print_cmd(struct cmd* cmd);
void print_diff_exit(struct cmd* cmd, char** expected, int argc);
bool equal_string(char* s, char* d);

/* ======== scannner ===== */
bool scanner_has_next();
char scanner_next();
void scanner_init(char command[]);
bool scanner_peek_equal(char* s);
char* scanner_get_token();
void scanner_consume(char c, char* msg_err);

/* ======== parse ===== */
struct cmd* parse_line();
struct cmd* parse_pipe();
struct cmd* parse_exec();

#endif