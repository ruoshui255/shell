#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void panic(char* msg);

static struct {
    char* p_start;
    char* p_current;
    char* p_end;
    // save the char that modity to '\0' in get_token
    char pre_char;
}scanner;


static char whitespace[] = " \t\r\n";
static char symbols[] = "<|>&;()";


void 
scanner_init(char* command) {
    scanner.p_start = command;
    scanner.p_current = command;
    scanner.p_end = command + strlen(command);
    scanner.pre_char = 0;
}

static void
scanner_skip_whitespace() {
    char* p = scanner.p_current;
    while (strchr(whitespace, *p)) {
        p++;
    }
    scanner.p_current = p;
}

bool
scanner_has_next() {
    if (scanner.pre_char != 0 || scanner.p_current < scanner.p_end) {
        return true;
    }
    return false;
}

char 
scanner_next() {
    if (scanner.pre_char != 0) {
        char c = scanner.pre_char;
        scanner.pre_char = 0;
        return c;
    }

    scanner_skip_whitespace();

    if (scanner.p_current < scanner.p_end) {
        char c = *scanner.p_current;
        scanner.p_current++;
        return c; 
    }

    // it can't arrive here;
    panic("next: it has arrive the end");
    return 0;
}

void 
scanner_consume(char c, char* msg_err) {
    if (!scanner_has_next()) {
        panic("consume: it has arrive the end");
    }

    char t = scanner_next();
    if (t != c) {
        fprintf(stderr, "%s: expected [%c] source [%c]\n", msg_err, c, t);
        exit(-1);
    }
    return;
}

bool
scanner_peek_equal(char* s){
    if (scanner.pre_char != 0) {
        return strchr(s, scanner.pre_char);
    }
    
    scanner_skip_whitespace();

    if (scanner_has_next()) {
        char c = *scanner.p_current;
        return strchr(s, c);
    }
    return false;
}


char *
scanner_get_token(){
    // cosume symbols
    if (scanner.pre_char != 0) {
        scanner.pre_char = 0;
        return NULL;
    }

    scanner_skip_whitespace();
   
   // 处理输入命令行中带有引号: grep "main" ./spin.c
    char* p = scanner.p_current;
    if (p < scanner.p_end && *p == '"') {
        scanner.p_current++;
        p++;
    }

    while (p < scanner.p_end && !strchr(whitespace, *p) && !strchr(symbols, *p)) {
        p++;
    }
    
    // terminate token: grep "main" ./spin.c
    if (p[-1] == '"') {
        p[-1] = '\0';
    }
    
    // string null terminate
    if (strchr(symbols, *p)) {
        scanner.pre_char = *p;        
    }
    *p = '\0';
    p++;
   
    char* ret = scanner.p_current;
    scanner.p_current = p;
    return ret;
}
