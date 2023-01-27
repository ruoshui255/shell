#ifndef __SCANNER 
#define __SCANNER

#include <stdbool.h>


typedef enum {
    TokenTypePipe,
    TokenTypeRedirectionWrite, 
    TokenTypeRedirectionWriteAppend,
    TokenTypeRedirectionRead,
    TokenTypeLeftParen, 
    TokenTypeRightParen,
    TokenTypeSemicolon,
    TokenTypeBackTask,
    TokenTypeAnd,
    
    // args
    TokenTypeArg,
    TokenTypeEOF,

    TokenTypeError,
}TokenType;

typedef struct {
    TokenType type;
    char* start;
    int length;
}Token;


/* ======== scannner ===== */
void scannerInit(char command[]);
Token scannerGetToken();

#endif