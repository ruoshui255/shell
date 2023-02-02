#ifndef __PARSE
#define __PARSE

#include <stdbool.h>

#define MAXARGS 10

typedef enum{
    CmdTypeExec,
    CmdTypeRedir,
    CmdTypePipe,
    CmdTypeList,
    CmdTypeBack,
    CmdTypeAnd,
}CmdType;

struct Cmd {
    struct Cmd* next;
    CmdType type;
};

struct CmdExec {
    struct Cmd* next;
    CmdType type;
    char* argv[MAXARGS];
    char* eargv[MAXARGS];
    int argc;
};

struct CmdRedir {
    struct Cmd* next;
    CmdType type;
    struct Cmd* cmd;
    char* file;
    char* efile;
    int mode;
    int fd;
};

struct CmdPipe {
    struct Cmd* next;
    CmdType type;
    struct Cmd* left;
    struct Cmd* right;
};

struct CmdList {
    struct Cmd* next;
    CmdType type;
    struct Cmd* left;
    struct Cmd* right;
};

struct CmdBack {
    struct Cmd* next;
    CmdType type;
    struct Cmd* cmd;
};

struct CmdAnd {
    struct Cmd* next;
    CmdType type;
    struct Cmd* left;
    struct Cmd* right;
};

/* ======== utils ===== */
void panic(char* msg);
// void print_arr(char* prefix, char* array[], int argc);
// void print_cmd(struct Cmd* cmd);
// void print_diff_exit(struct Cmd* cmd, char** expected, int argc);
// bool equal_string(char* s, char* d);

/* ======== parse ===== */
struct Cmd* cmdParse(char* buf);

#endif